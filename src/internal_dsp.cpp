#include <internal_dsp.hpp>
#include <cmath>

void guitar_amp::dsp::hardclip(const float *input, float *transform, float threshold, ma_uint32 frameCount) {
    for (ma_uint32 i = 0; i < frameCount; i++) {
        if (input[i] > 0) {
            transform[i] = std::min(input[i]*10, threshold);
        } else {
            transform[i] = std::max(input[i]*10, -threshold);
        }
    }
}