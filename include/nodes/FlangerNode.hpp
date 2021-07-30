#ifndef GUITAR_AMP_FLANGER_NODE_HPP
#define GUITAR_AMP_FLANGER_NODE_HPP

#define MAX_POSSIBLE_DELAY_MS 50.0f 

#include "MiddleNode.hpp"
#include "state.hpp"
#include "headers.hpp"
#include "internal_dsp.hpp"

namespace guitar_amp {
    class FlangerNode : public MiddleNode {
    public:
        
        FlangerNode(int id, const AudioInfo current_audio_info);
        ~FlangerNode();

        void showGui();
        void ApplyFX(const float *in, float *out, size_t numFrames, AudioInfo info);

    private:
    
        dsp::ring_buffer<float> rb;
        float delay_frequency = 0.5f; // measured in Hz
        
        float min_delay_time = 1.0f; // 1 millisecond
        float max_delay_time = 10.0f;
        float feedback_gain = -6.0f;

        float internal_timer = 0.0f;

        size_t min_delay_samples;
        size_t max_delay_samples;

    };
}

#endif