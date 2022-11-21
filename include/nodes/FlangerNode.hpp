#ifndef GUITAR_AMP_FLANGER_NODE_HPP
#define GUITAR_AMP_FLANGER_NODE_HPP

#define MAX_POSSIBLE_DELAY_MS 100.0f

#include "MiddleNode.hpp"
#include "state.hpp"
#include "headers.hpp"
#include "internal_dsp.hpp"
#include <mindsp/util/ring_buffer.hpp>

namespace guitar_amp {
    class FlangerNode : public MiddleNode {
    public:
        
        FlangerNode(int id, const AudioInfo current_audio_info);
        FlangerNode(int id, const AudioInfo current_audio_info, const sol::table &init_table);
        virtual ~FlangerNode();

        void showGui();
        void ApplyFX(const float *in, float *out, size_t numFrames, AudioInfo info);
        void luaInit(const sol::table &init_table);
        virtual sol::table serializeLua();

    private:
    
        mindsp::util::ring_buffer<float> rb;    
        float delay_frequency = 1.0f; // measured in Hz
        
        float avg_delay_time = 5.f;
        float delay_range_time = 2.f; // 5 +- 2 

        float feedback_gain = -6.0f;

        float internal_timer = 0;

        size_t max_delay_samples = 0;
        size_t min_delay_samples = 0;

        float depth = 50;

        bool changed_delay = false;

    };
}

#endif