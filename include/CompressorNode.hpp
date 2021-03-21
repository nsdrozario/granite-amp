#ifndef GUITAR_AMP_COMPRESSORNODE_HPP
#define GUITAR_AMP_COMPRESSORNODE_HPP
#include "MiddleNode.hpp"
#include "internal_dsp.hpp"

namespace guitar_amp {
    class CompressorNode : public MiddleNode {
    public:
        CompressorNode(int id);
        ~CompressorNode();    

        void showGui();
        void ApplyFX(const float *in, float *out, size_t numFrames);

    private:
        // static const char COMPRESSOR_RMS = 1<<0;
        static const char COMPRESSOR_PEAK = 1<<1;
        // might add more metering types later
        
        // times are in seconds
        float attack = 0.0f;
        float release = 0.0f;
        float ratio = 2.0f;
        float threshold = -6.0f; // dB
        char metering = CompressorNode::COMPRESSOR_PEAK;

        bool sidechain_enabled = false;

    };
}

#endif