#ifndef GUITAR_AMP_DELAY_NODE_HPP
#define GUITAR_AMP_DELAY_NODE_HPP
#include "MiddleNode.hpp"
#include "state.hpp"
#include "headers.hpp"
#include "internal_dsp.hpp"

namespace guitar_amp {
    class DelayNode : public MiddleNode {
    public:
        DelayNode(int id);
        ~DelayNode();

        void showGui();
        void ApplyFX(const float *in, float *out, size_t num_frames);

    private:
        float time_delay;
        size_t period_size;
        float *ring_buffer;
        size_t write_ptr = 0;
        size_t read_ptr = 0;
        size_t samples_delay = 0;
        size_t inc_write_ptr();
        size_t inc_read_ptr();
        
    };
}

#endif