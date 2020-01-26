#ifndef STOMPBOX_BEATTRACKER_H
#define STOMPBOX_BEATTRACKER_H

#include "CircularBuffer.h"
#include "NE10.h"
#include "OnsetDetection.h"
#include <array>
#include <atomic>
#include <complex>
#include <oboe/Definitions.h>
#include <vector>

/*
 * A re-implementation of https://github.com/adamstark/BTrack
 */
namespace btrack {

class BeatTracker {

private:
	static constexpr size_t OnsetDFBufferSize = 512;
	static constexpr size_t FFTLengthForACFCalculation = 1024;
	static constexpr float Tightness = 5.0F;
	static constexpr float Alpha = 0.1F;
	static constexpr float Epsilon = 0.0001F;
	static constexpr float OnsetThreshold = 100.0F;

	int32_t sampleRate;

	circbuf::CircularBuffer onsetDF;
	circbuf::CircularBuffer cumulativeScore;
	onset::OnsetDetectionFunction odf;

	std::vector<ne10_fft_cpx_float32_t> complexIn;
	std::vector<ne10_fft_cpx_float32_t> complexOut;
	ne10_fft_cfg_float32_t acfFFT;

	float tempoToLagFactor;
	float latestCumulativeScoreValue;
	float beatPeriod;
	int m0;
	int beatCounter;

	int discardSamples;

	std::array<float, 512> acf;
	std::array<float, 128> combFilterBankOutput;
	std::array<float, 41> tempoObservationVector;
	std::array<float, 41> delta;
	std::array<float, 41> prevDelta;

	void processOnsetDetectionFunctionSample(float sample);
	void updateCumulativeScore(float odfSample);
	void predictBeat();
	void calculateTempo();
	void adaptiveThreshold(float* x, size_t N);
	void calculateOutputOfCombFilterBank();
	void normalizeArray(float* x, size_t N);
	float calculateMeanOfArray(const float* x, size_t start, size_t end);
	void calculateBalancedACF(std::vector<float>& onsetDetectionFunction);

public:
	bool beatDueInFrame;
	float estimatedTempo;

	BeatTracker(int32_t sampleRate);
	~BeatTracker();

	void processCurrentFrame(std::vector<float> samples);
};
} // namespace btrack

#endif // STOMPBOX_BEATTRACKER_H
