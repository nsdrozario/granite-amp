#ifndef GUITAR_AMP_THREEBANDEQ_HPP
#define GUITAR_AMP_THREEBANDEQ_HPP

#include <MiddleNode.hpp>
#include <internal_dsp.hpp>
#include <miniaudio.h>

namespace guitar_amp {
    class ThreeBandEQ : public MiddleNode {
    public:
        ThreeBandEQ(int id, const AudioInfo current_audio_info);
        virtual ~ThreeBandEQ();

        void showGui();
        void ApplyFX(const float *in, float *out, size_t numFrames, AudioInfo info);

    private:

        float bass_gain = 0.0f;
        float treble_gain = 0.0f;
        float mid_gain = 0.0f;

        ma_hishelf2 high_shelf;
        ma_loshelf2 low_shelf;
        ma_peak2 midrange;

        ma_hishelf2_config hishelf_config;
        ma_loshelf2_config loshelf_config;
        ma_peak2_config mid_config;
    };
}

#endif