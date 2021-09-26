#ifndef GUITAR_AMP_THREEBANDEQ_HPP
#define GUITAR_AMP_THREEBANDEQ_HPP

#include <MiddleNode.hpp>
#include <internal_dsp.hpp>
#include <miniaudio.h>
#include <mindsp/filter.hpp>

namespace guitar_amp {
    class ThreeBandEQ : public MiddleNode {
    public:
        ThreeBandEQ(int id, const AudioInfo current_audio_info);
        ThreeBandEQ(int id, const AudioInfo current_audio_info, const sol::table &init_table);
        virtual ~ThreeBandEQ();

        void showGui();
        void ApplyFX(const float *in, float *out, size_t numFrames, AudioInfo info);
        void luaInit(const sol::table &init_table);

    private:

        float bass_gain = 0.0f;
        float treble_gain = 0.0f;
        float mid_gain = 0.0f;

        mindsp::filter::biquad_filter low_shelf;
        mindsp::filter::biquad_filter high_shelf;
        mindsp::filter::biquad_filter mid_range;
        
    };
}

#endif