#ifndef GUITAR_AMP_FLANGER_NODE_HPP
#define GUITAR_AMP_FLANGER_NODE_HPP

#define MAX_POSSIBLE_DELAY_MS 30.0f

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
        float delay_frequency = 1.0f; // measured in Hz
        
        float min_delay_time = 1.0f; // 1 millisecond
        float max_delay_time = 1.0f;
        float feedback_gain = -6.0f;

        float internal_timer = 0;

        size_t max_delay_samples = 0;

        // honestly using an inbuilt ring buffer may be easier here
        float *delay_buf = nullptr;
        size_t delay_buf_size = 0;
        // read ptr position will have to be continually calculated so no point in storing it
        size_t write_ptr = 0;

    };
}

#endif