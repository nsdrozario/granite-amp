#ifndef GUITAR_AMP_DELAY_NODE_HPP
#define GUITAR_AMP_DELAY_NODE_HPP
#include "MiddleNode.hpp"
#include "state.hpp"
#include "headers.hpp"
#include "internal_dsp.hpp"

namespace guitar_amp {
    class DelayNode : public MiddleNode {
    public:
        DelayNode(int id, const AudioInfo current_audio_info);
        DelayNode(int id, const AudioInfo current_audio_info, const sol::table &init_table);
        virtual ~DelayNode();

        void showGui();
        void ApplyFX(const float *in, float *out, size_t numFrames, AudioInfo info);
        void luaInit(const sol::table &init_table);
        virtual sol::table serializeLua();

    private:

        dsp::ring_buffer<float> buf;

        float time_delay = 0.0f;
        // about 50% feedback
        float delay_gain = -6.0f; 
        float feedback_gain = -6.0f;
        size_t samples_delay = 0;
        
    };
}

#endif