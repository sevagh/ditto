#ifndef STOMPBOX_ONSETDETECTION_H
#define STOMPBOX_ONSETDETECTION_H

#include "NE10.h"
#include "stompbox/MiscUtil.h"
#include "stompbox/Window.h"
#include <array>
#include <cmath>

namespace stompbox::onset_detection {
namespace detail {
inline float princarg(float phaseVal)
{
    // if phase value is less than or equal to -pi then add 2*pi
    while (phaseVal <= (-stompbox::math_neon::PI))
    {
        phaseVal += 2 * stompbox::math_neon::PI;
    }

    // if phase value is larger than pi, then subtract 2*pi
    while (phaseVal > stompbox::math_neon::PI)
    {
        phaseVal -= 2 * stompbox::math_neon::PI;
    }

    return phaseVal;
}

}
enum OnsetDetectionFunctionType
{
    EnergyEnvelope,
    EnergyDifference,
    SpectralDifference,
    SpectralDifferenceHWR,
    PhaseDeviation,
    ComplexSpectralDifference,
    ComplexSpectralDifferenceHWR,
    HighFrequencyContent,
    HighFrequencySpectralDifference,
    HighFrequencySpectralDifferenceHWR
};

template <size_t FrameSize,
          size_t HopSize,
          stompbox::window::WindowType WindowType = stompbox::window::WindowType::HanningWindow
        >
class OnsetDetectionFunction {
	static_assert(stompbox::detail::misc_util::is_half(FrameSize, HopSize));

public:
	OnsetDetectionFunction()
        : OnsetDetectionFunction(ComplexSpectralDifferenceHWR)
    {};

	explicit OnsetDetectionFunction(OnsetDetectionFunctionType t)
        : t(t)
        , fftCfg(ne10_fft_alloc_r2c_float32(FrameSize))
        , prevEnergySum(0.0F){
	};

	~OnsetDetectionFunction() {
        ne10_fft_destroy_r2c_float32(fftCfg);
	};

	float calculate_sample(std::vector<float>& buffer) {
        // shift audio samples back in frame by hop size
        std::copy(frame.begin() + HopSize, frame.end(), frame.begin());

        // add new samples to frame from input buffer
        std::copy(buffer.begin(), buffer.begin() + HopSize, frame.end() - HopSize);

        switch (t) {
        case EnergyEnvelope:
            return energy_envelope();
        case EnergyDifference:
            return energy_difference();
        case SpectralDifference:
            return spectral_difference();
        case SpectralDifferenceHWR:
            return spectral_difference_hwr();
        case PhaseDeviation:
            return phase_deviation();
        case ComplexSpectralDifference:
            return complex_spectral_difference();
        case ComplexSpectralDifferenceHWR:
            return complex_spectral_difference_hwr();
        case HighFrequencyContent:
            return high_frequency_content();
        case HighFrequencySpectralDifference:
            return high_frequency_spectral_difference();
        case HighFrequencySpectralDifferenceHWR:
            return high_frequency_spectral_difference_hwr();
        default:
            return 1.0;
        }
	};

private:
	void perform_FFT()
	{
		size_t fsize2 = HopSize;

		for (size_t i = 0; i < fsize2; ++i) {
			std::iter_swap(frame.begin() + i, frame.begin() + i + fsize2);
			frame[i] *= window.data[i + fsize2];
			frame[i + fsize2] *= window.data[i];
		}

		ne10_fft_r2c_1d_float32_neon(complexOut.data(), frame.data(), fftCfg);
	};

    float energy_envelope() {
        float sum = 0.0F;

        std::for_each(frame.begin(), frame.end(), [&](float x) { sum += x * x; });

        return sum;
    };

    float energy_difference() {
        float sum = 0.0F;

        std::for_each(frame.begin(), frame.end(), [&](float x) { sum += x * x; });

        float sample
            = sum - prevEnergySum; // sample is first order difference in energy
        prevEnergySum = sum;       // store energy value for next calculation

        if (sample > 0.0F) {
            return sample; // return difference
        }
        return 0.0F;
    };

    float spectral_difference() {
        float diff = 0.0F;
        float sum = 0.0F;

        // perform the FFT
        perform_FFT();

        // compute first (N/2)+1 mag values
        for (size_t i = 0; i < HopSize + 1; i++) {
            magSpec[i] = std::sqrtf(std::powf(complexOut[i].r, 2) + std::powf(complexOut[i].i, 2));
        }

        // mag spec symmetric above (N/2)+1 so copy previous values
        for (size_t i = HopSize + 1; i < FrameSize; ++i) {
            magSpec[i] = magSpec[FrameSize - i];
        }

        for (size_t i = 0; i < FrameSize; ++i) {
            // calculate difference
            diff = magSpec[i] - prevMagSpec[i];

            // ensure all difference values are positive
            if (diff < 0) {
                diff = diff * -1;
            }

            // add difference to sum
            sum += diff;

            // store magnitude spectrum bin for next detection function sample calculation
            prevMagSpec[i] = magSpec[i];
        }

        return sum;
    };

    float spectral_difference_hwr() {
        float diff = 0.0F;
        float sum = 0.0F;

        // perform the FFT
        perform_FFT();

        // compute first (N/2)+1 mag values
        for (size_t i = 0; i < HopSize + 1; ++i)
        {
            magSpec[i] = std::sqrtf(std::powf(complexOut[i].r,2) + std::powf(complexOut[i].i, 2));
        }
        // mag spec symmetric above (N/2)+1 so copy previous values
        for (size_t i = HopSize+1; i < FrameSize; ++i)
        {
            magSpec[i] = magSpec[FrameSize-i];
        }

        for (size_t i = 0; i < FrameSize; ++i)
        {
            // calculate difference
            diff = magSpec[i] - prevMagSpec[i];

            // only add up positive differences
            if (diff > 0)
            {
                // add difference to sum
                sum += diff;
            }

            // store magnitude spectrum bin for next detection function sample calculation
            prevMagSpec[i] = magSpec[i];
        }

        return sum;
    };

    float phase_deviation() {
        float dev;
        float pdev;
        float sum = 0.0F;

        // perform the FFT
        perform_FFT();

        // compute phase values from fft output and sum deviations
        for (size_t i = 0; i < FrameSize; ++i)
        {
            // calculate phase value
            phase[i] = std::atan2f(complexOut[i].i, complexOut[i].r);

            // calculate magnitude value
            magSpec[i] = std::sqrtf(std::powf(complexOut[i].r,2) + std::powf(complexOut[i].i,2));

            // if bin is not just a low energy bin then examine phase deviation
            if (magSpec[i] > 0.1)
            {
                dev = phase[i] - (2*prevPhase[i]) + prevPhase2[i];	// phase deviation
                pdev = detail::princarg(dev);	// wrap into [-pi,pi] range

                // make all values positive
                if (pdev < 0)
                {
                    pdev = pdev*-1;
                }

                // add to sum
                sum += pdev;
            }

            // store values for next calculation
            prevPhase2[i] = prevPhase[i];
            prevPhase[i] = phase[i];
        }

        return sum;
    };

	float complex_spectral_difference_hwr() {
        float phaseDeviation;
        float sum;
        float magnitudeDifference;
        float csd;

        // perform the FFT
        perform_FFT();

        sum = 0; // initialise sum to zero

        // compute phase values from fft output and sum deviations
        for (size_t i = 0; i < FrameSize; ++i) {
            // calculate phase value
            phase[i] = std::atan2f(complexOut[i].i, complexOut[i].r);

            // calculate magnitude value
            magSpec[i] = std::sqrtf(std::powf(complexOut[i].r, 2) + std::powf(complexOut[i].i, 2));

            // phase deviation
            phaseDeviation = phase[i] - (2 * prevPhase[i]) + prevPhase2[i];

            // calculate magnitude difference (real part of Euclidean distance
            // between complex frames)
            magnitudeDifference = magSpec[i] - prevMagSpec[i];

            // if we have a positive change in magnitude, then include in sum,
            // otherwise ignore (half-wave rectification)
            if (magnitudeDifference > 0) {
                // calculate complex spectral difference for the current spectral bin
                csd = std::sqrtf(std::powf(magSpec[i], 2) + std::powf(prevMagSpec[i], 2)
                            - 2 * magSpec[i] * prevMagSpec[i]
                              * std::cosf(phaseDeviation));

                // add to sum
                sum = sum + csd;
            }

            // store values for next calculation
            prevPhase2[i] = prevPhase[i];
            prevPhase[i] = phase[i];
            prevMagSpec[i] = magSpec[i];
        }

        return sum;
	};

    float complex_spectral_difference() {
        float phaseDeviation;
        float sum;
        float csd;

        // perform the FFT
        perform_FFT();

        sum = 0; // initialise sum to zero

        // compute phase values from fft output and sum deviations
        for (size_t i = 0; i < FrameSize; ++i)
        {
            // calculate phase value
            phase[i] = std::atan2f(complexOut[i].i, complexOut[i].r);

            // calculate magnitude value
            magSpec[i] = std::sqrtf(std::powf(complexOut[i].r,2) + std::powf(complexOut[i].i,2));

            // phase deviation
            phaseDeviation = phase[i] - (2 * prevPhase[i]) + prevPhase2[i];

            // calculate complex spectral difference for the current spectral bin
            csd = sqrt (std::powf(magSpec[i], 2) + std::powf(prevMagSpec[i], 2) - 2 * magSpec[i] * prevMagSpec[i] * cos (phaseDeviation));

            // add to sum
            sum = sum + csd;

            // store values for next calculation
            prevPhase2[i] = prevPhase[i];
            prevPhase[i] = phase[i];
            prevMagSpec[i] = magSpec[i];
        }

        return sum;
    };

    float high_frequency_content() {
        float sum;

        // perform the FFT
        perform_FFT();

        sum = 0; // initialise sum to zero

        // compute phase values from fft output and sum deviations
        for (size_t i = 0; i < FrameSize; ++i)
        {
            // calculate magnitude value
            magSpec[i] = std::sqrtf(std::powf(complexOut[i].r, 2) + std::powf(complexOut[i].i,2));

            sum = sum + (magSpec[i] * ((float) (i+1)));

            // store values for next calculation
            prevMagSpec[i] = magSpec[i];
        }

        return sum;
    };

    float high_frequency_spectral_difference() {
        float sum;
        float mag_diff;

        // perform the FFT
        perform_FFT();

        sum = 0; // initialise sum to zero

        // compute phase values from fft output and sum deviations
        for (size_t i = 0; i < FrameSize; ++i)
        {
            // calculate magnitude value
            magSpec[i] = std::sqrtf(std::powf(complexOut[i].r, 2) + std::powf(complexOut[i].i, 2));

            // calculate difference
            mag_diff = magSpec[i] - prevMagSpec[i];

            if (mag_diff < 0)
            {
                mag_diff = -mag_diff;
            }

            sum = sum + (mag_diff * ((float) (i+1)));

            // store values for next calculation
            prevMagSpec[i] = magSpec[i];
        }

        return sum;
    };

    float high_frequency_spectral_difference_hwr() {
        float sum;
        float mag_diff;

        // perform the FFT
        perform_FFT();

        sum = 0; // initialise sum to zero

        // compute phase values from fft output and sum deviations
        for (size_t i = 0; i < FrameSize; ++i)
        {
            // calculate magnitude value
            magSpec[i] = std::sqrtf(std::powf(complexOut[i].r,2) + std::powf (complexOut[i].i, 2));

            // calculate difference
            mag_diff = magSpec[i] - prevMagSpec[i];

            if (mag_diff > 0)
            {
                sum = sum + (mag_diff * ((float) (i+1)));
            }

            // store values for next calculation
            prevMagSpec[i] = magSpec[i];
        }

        return sum;
    };

	OnsetDetectionFunctionType t;
	static constexpr stompbox::window::Window<FrameSize> window = stompbox::window::get_window<FrameSize, WindowType>();

	std::array<ne10_fft_cpx_float32_t, FrameSize> complexOut = {};
    ne10_fft_r2c_cfg_float32_t fftCfg;

	std::array<float, FrameSize> frame = {};
	std::array<float, FrameSize> magSpec = {};
	std::array<float, FrameSize> prevMagSpec = {};
	std::array<float, FrameSize> phase = {};
	std::array<float, FrameSize> prevPhase = {};
	std::array<float, FrameSize> prevPhase2 = {};

	float prevEnergySum;
};
} // namespace stompbox::onset_detection

#endif // STOMPBOX_ONSETDETECTION_H
