#ifndef GUITAR_AMP_INTERNAL_DSP_HPP
#define GUITAR_AMP_INTERNAL_DSP_HPP

extern "C" {
    #include "miniaudio.h"
}

namespace guitar_amp {
    namespace dsp {
        void hardclip(const float *input, float *transform, float threshold, ma_uint32 frameCount);
    }
}

#endif