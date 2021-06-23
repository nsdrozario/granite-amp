#ifndef GUITAR_AMP_CAB_SIM_NODE_HPP
#define GUITAR_AMP_CAB_SIM_NODE_HPP

#include "headers.hpp"
#include "internal_dsp.hpp"
#include "state.hpp"
#include "MiddleNode.hpp"

#include <FFTConvolver.h>

namespace guitar_amp {
    /* 
        This is NOT replacing the convolver! This is an algorithmic cabinet simulation so that you
        don't necessarily need to obtain your own impulse responses to get a functional sound.
    */
    class CabSimNode : public MiddleNode {
    public:
        CabSimNode(int id);
        ~CabSimNode();

        void showGui();
        void ApplyFX(const float *in, float *out, size_t numFrames);
        
    private:

        fftconvolver::FFTConvolver convolver;
        size_t last_period_size;

    };
}

#endif