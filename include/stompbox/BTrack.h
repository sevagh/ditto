#ifndef STOMPBOX_BTRACK_H
#define STOMPBOX_BTRACK_H

#include "../src/BTrackPrecomputed.h"
#include "CircularBuffer.h"
#include "NE10.h"
#include "stompbox/OnsetDetection.h"
#include "stompbox/MathNeon.h"
#include <array>
#include <complex>
#include <cstddef>
#include <vector>

namespace stompbox::btrack {
template <std::size_t FrameSize, std::size_t HopSize>
class BTrack {
private:
	static constexpr size_t OnsetDFBufferSize = 512;
	static constexpr size_t FFTLengthForACFCalculation = 1024;
	static constexpr float Tightness = 5.0F;
	static constexpr float Alpha = 0.1F;
	static constexpr float Epsilon = 0.0001F;

	int sampleRate;

	stompbox_private::circular_buffer::CircularBuffer<OnsetDFBufferSize> onsetDF
	    = {};
	stompbox_private::circular_buffer::CircularBuffer<OnsetDFBufferSize> cumulativeScore
	    = {};
	stompbox::onset_detection::OnsetDetectionFunction<FrameSize, HopSize> odf;

	std::array<ne10_fft_cpx_float32_t, FFTLengthForACFCalculation> complexIn
	    = {};
	std::array<ne10_fft_cpx_float32_t, FFTLengthForACFCalculation> complexOut
	    = {};
	ne10_fft_cfg_float32_t acfFFT;

	float tempoToLagFactor;
	float latestCumulativeScoreValue;
	float beatPeriod;
	int m0;
	int beatCounter;

	int discardSamples;

	std::array<float, 512> acf = {};
	std::array<float, 128> combFilterBankOutput = {};
	std::array<float, 41> tempoObservationVector = {};
	std::array<float, 41> delta = {};
	std::array<float, 41> prevDelta = {};

	void processOnsetDetectionFunctionSample(float sample)
	{
		sample = fabs(sample) + Epsilon;
		m0--;
		beatCounter--;
		beatDueInFrame = false;

		onsetDF.append(sample);
		updateCumulativeScore(sample);

		if (m0 == 0) {
			predictBeat();
		}

		if (beatCounter == 0) {
			beatDueInFrame = true;
			// resampleOnsetDetectionFunction();
			calculateTempo();
		}
	};

	void updateCumulativeScore(float odfSample)
	{
		size_t start;
		size_t end;
		size_t winsize;
		float max;

		start = (size_t)(OnsetDFBufferSize - roundf(2.0F * beatPeriod));
		end = (size_t)(OnsetDFBufferSize - roundf(beatPeriod / 2.0F));

		winsize = end - start + 1;

		float w1[winsize];
		float v = -2.0F * beatPeriod;
		float wcumscore;

		// create window
		for (size_t i = 0; i < winsize; ++i) {
			// TODO replace with faster MathNeon computations
			w1[i] = expf((-1 * powf(Tightness * logf(-v / beatPeriod), 2.0F))
			             / 2.0F);
			v += 1.0F;
		}

		max = 0;
		size_t n = 0;
		for (size_t i = start; i <= end; ++i) {
			wcumscore = cumulativeScore[i] * w1[n++];
			if (wcumscore > max)
				max = wcumscore;
		}

		latestCumulativeScoreValue
		    = ((1.0F - Alpha) * odfSample) + (Alpha * max);
		cumulativeScore.append(latestCumulativeScoreValue);
	};

	void predictBeat()
	{
		auto windowSize = ( size_t )beatPeriod;
		float futureCumulativeScore[OnsetDFBufferSize + windowSize];
		float w2[windowSize];

		// copy cumscore to first part of fcumscore
		for (size_t i = 0; i < OnsetDFBufferSize; ++i) {
			futureCumulativeScore[i] = cumulativeScore[i];
		}

		// create future window - TODO can this be precomputed?
		float v = 1.0F;
		for (size_t i = 0; i < windowSize; ++i) {
			w2[i] = expf((-1.0F * powf((v - (beatPeriod / 2.0F)), 2.0F))
			             / (2.0F * powf((beatPeriod / 2.0F), 2.0F)));
			v += 1.0F;
		}

		// create past window
		v = -2.0F * beatPeriod;
		auto start = (size_t)(OnsetDFBufferSize - roundf(2.0F * beatPeriod));
		auto end = (size_t)(OnsetDFBufferSize - roundf(beatPeriod / 2.0F));
		size_t pastwinsize = end - start + 1;
		float w1[pastwinsize];

		// TODO replace logs, exp, power, etc. with MathNeon faster versions
		for (size_t i = 0; i < pastwinsize; ++i) {
			w1[i] = expf((-1.0F * powf(Tightness * logf(-v / beatPeriod), 2.0F))
			             / 2.0F);
			v = v + 1;
		}

		// calculate future cumulative score
		float max;
		int n;
		float wcumscore;
		for (size_t i = OnsetDFBufferSize;
		     i < (OnsetDFBufferSize + windowSize); ++i) {
			start = (size_t)(i - roundf(2.0F * beatPeriod));
			end = (size_t)(i - roundf(beatPeriod / 2.0F));

			max = 0;
			n = 0;
			for (size_t k = start; k <= end; ++k) {
				wcumscore = futureCumulativeScore[k] * w1[n];

				if (wcumscore > max) {
					max = wcumscore;
				}
				n++;
			}

			futureCumulativeScore[i] = max;
		}

		// predict beat
		max = 0;
		n = 0;

		for (size_t i = OnsetDFBufferSize;
		     i < (OnsetDFBufferSize + windowSize); ++i) {
			wcumscore = futureCumulativeScore[i] * w2[n];

			if (wcumscore > max) {
				max = wcumscore;
				beatCounter = n;
			}

			n++;
		}

		m0 = ( int )(beatCounter + roundf(beatPeriod / 2.0F));
	};

	void calculateTempo()
	{
		// adaptive threshold on input - TODO faster math with MathNeon
		stompbox::math_neon::AdaptiveThreshold(onsetDF.buffer.data(), onsetDF.buffer.size());

		// calculate auto-correlation function of detection function
		calculateBalancedACF(onsetDF.buffer);

		// calculate output of comb filterbank - TODO faster math with MathNeon
		calculateOutputOfCombFilterBank();

		// adaptive threshold on rcf
		stompbox::math_neon::AdaptiveThreshold(
		    combFilterBankOutput.data(), combFilterBankOutput.size());

		size_t t_index;
		size_t t_index2;
		// calculate tempo observation vector from beat period observation vector
		for (size_t i = 0; i < 41; ++i) {
			t_index
			    = ( size_t )roundf(tempoToLagFactor / (((2.0F * i) + 80.0F)));
			t_index2
			    = ( size_t )roundf(tempoToLagFactor / (((4.0F * i) + 160.0F)));

			tempoObservationVector[i] = combFilterBankOutput[t_index - 1]
			                            + combFilterBankOutput[t_index2 - 1];
		}

		float maxval;
		float maxind;
		float curval;

		for (size_t j = 0; j < 41; ++j) {
			maxval = -1;
			for (size_t i = 0; i < 41; ++i) {
				curval = prevDelta[i]
				         * detail::precomputed::TempoTransitionMatrix[i][j];

				if (curval > maxval) {
					maxval = curval;
				}
			}

			delta[j] = maxval * tempoObservationVector[j];
		}

		stompbox::math_neon::NormalizeArray(delta.data(), 41);

		maxind = -1;
		maxval = -1;

		for (size_t j = 0; j < 41; ++j) {
			if (delta[j] > maxval) {
				maxval = delta[j];
				maxind = j;
			}

			prevDelta[j] = delta[j];
		}

		beatPeriod = roundf((60.0F * (( float )sampleRate))
		                    / (((2.0F * maxind) + 80.0F) * (( float )HopSize)));

		if (beatPeriod > 0) {
			estimatedTempo
			    = 60.0F
			      / (((( float )HopSize) / (( float )sampleRate)) * beatPeriod);
		}
	};

	void calculateOutputOfCombFilterBank()
	{
		std::fill(
		    combFilterBankOutput.begin(), combFilterBankOutput.end(), 0.0F);

		for (int i = 2; i <= 127; ++i) {   // max beat period
			for (int a = 1; a <= 4; ++a) { // number of comb elements
				for (int b = 1 - a;
				     b <= a - 1; ++b) { // general state using normalisation of
					                    // comb elements
					combFilterBankOutput[i - 1]
					    = combFilterBankOutput[i - 1]
					      + (acf[(a * i + b) - 1]
					         * detail::precomputed::RayleighWeightingVector128[i - 1])
					            / (2 * a - 1); // calculate value for comb
					                           // filter row
				}
			}
		}
	};



	void calculateBalancedACF(
	    std::array<float, OnsetDFBufferSize>& onsetDetectionFunction)
	{
		// copy 512 samples of onsetDetection into 1024 float
		for (size_t i = 0; i < OnsetDFBufferSize; ++i) {
			complexIn[i].r = onsetDetectionFunction[i];
			complexIn[i].i = 0.0F;
		}

		// zero pad the remaining 512
		for (size_t i = OnsetDFBufferSize; i < FFTLengthForACFCalculation; ++i) {
			complexIn[i].r = 0.0F;
			complexIn[i].i = 0.0F;
		}

		ne10_fft_c2c_1d_float32_neon(
		    complexOut.data(), complexIn.data(), acfFFT, 0);

		// multiply by complex conjugate
		for (int i = 0; i < FFTLengthForACFCalculation; i++) {
			complexOut[i].r = complexOut[i].r * complexOut[i].r
			                  + complexOut[i].i * complexOut[i].i;
			complexOut[i].i = 0.0F;
		}

		// perform the ifft
		ne10_fft_c2c_1d_float32_neon(
		    complexIn.data(), complexOut.data(), acfFFT, 1);

		auto lag = ( float )OnsetDFBufferSize;

		for (size_t i = 0; i < OnsetDFBufferSize; i++) {
			float absValue = sqrtf(complexIn[i].r * complexIn[i].r
			                       + complexIn[i].i * complexIn[i].i);

			// divide by inverse lag to deal with scale bias towards small lags
			acf[i] = absValue / lag;

			// this division by 1024 is technically unnecessary but it ensures
			// the algorithm produces exactly the same ACF output as the old
			// time domain implementation. The time difference is minimal so I
			// decided to keep it acf[i] = acf[i] / ( float
			// )FFTLengthForACFCalculation;
			lag -= 1.0F;
		}
	};

public:
	bool beatDueInFrame;
	float estimatedTempo;

	explicit BTrack(
	    int sampleRate,
	    stompbox::onset_detection::OnsetDetectionFunctionType onsetType)
	    : sampleRate(sampleRate)
	    , odf(stompbox::onset_detection::OnsetDetectionFunction<FrameSize, HopSize>(
	          onsetType))
        , acfFFT(ne10_fft_alloc_c2c_float32_neon(FFTLengthForACFCalculation))
	    , tempoToLagFactor(60.0F * (( float )sampleRate) / ( float )HopSize)
	    , latestCumulativeScoreValue(0.0F)
	    , beatPeriod(roundf(
	          60.0F / (((( float )HopSize) / ( float )sampleRate) * 120.0F)))
	    , m0(10)
	    , beatCounter(-1)
	    , discardSamples(sampleRate / 2)
	    , beatDueInFrame(false)
	    , estimatedTempo(120.0F)
	{
		std::fill(prevDelta.begin(), prevDelta.end(), 1.0F);

		// initialise df_buffer
		for (size_t i = 0; i < OnsetDFBufferSize; ++i) {
			if ((i % (( size_t )round(beatPeriod)))
			    == 0) { // TODO faster modulo
				onsetDF[i] = 1.0F;
			}
		}
	};

	~BTrack()
	{
		// destroy FFT things here
		ne10_fft_destroy_c2c_float32(acfFFT);
	};

	void processCurrentFrame(std::vector<float> samples)
	{
		float sample = odf.calculate_sample(samples);
        processOnsetDetectionFunctionSample(sample);
	};
};
} // namespace stompbox::btrack

#endif // STOMPBOX_BTRACK_H
