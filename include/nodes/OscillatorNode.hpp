#ifndef GUITAR_AMP_OSCILLATOR_NODE_HPP
#define GUITAR_AMP_OSCILLATOR_NODE_HPP

#include "MiddleNode.hpp"
#include "headers.hpp"
#include "state.hpp"

namespace guitar_amp {
    class OscillatorNode : public MiddleNode {
    public:
        OscillatorNode(int id);
        virtual ~OscillatorNode();

        virtual void showGui();
        virtual void ApplyFX(const float *in, float *out, size_t numFrames, const AudioInfo &info);

    protected:
        float freq = 440.0f; // A4 = 440hz
        float amplitude = -144.0f; // in dBFS
        /*
        0: sine
        1: square
        2: triangle
        3: sawtooth
        */
        int wave_type;

        ma_waveform wav_gen;
    };
}

#endif