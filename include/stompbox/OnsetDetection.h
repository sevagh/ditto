#ifndef STOMPBOX_ONSETDETECTION_H
#define STOMPBOX_ONSETDETECTION_H

#include "MiscUtil.h"
#include "NE10.h"
#include <array>

namespace stompbox::onset_detection {
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

template <size_t FrameSize, size_t HopSize>
class OnsetDetectionFunction {
    static_assert(stompbox::misc_util::is_half(FrameSize, HopSize));

public:
	OnsetDetectionFunction();
    OnsetDetectionFunction(OnsetDetectionFunctionType t);
	~OnsetDetectionFunction();

	float calculate_sample(std::vector<float>& buffer);

private:
	float complexSpectralDifferenceHWR();

	OnsetDetectionFunctionType t;

	std::array<ne10_fft_cpx_float32_t, FrameSize> complexOut;
    ne10_fft_r2c_cfg_float32_t fftCfg;

	std::array<float, FrameSize> frame;
	std::array<float, FrameSize> magSpec;
	std::array<float, FrameSize> prevMagSpec;
	std::array<float, FrameSize> phase;
	std::array<float, FrameSize> prevPhase;
	std::array<float, FrameSize> prevPhase2;
};
} // namespace stompbox::onset_detection

#endif // STOMPBOX_ONSETDETECTION_H
