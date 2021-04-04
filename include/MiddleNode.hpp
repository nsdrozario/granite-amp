#ifndef GUITAR_AMP_MIDDLENODE_HPP
#define GUITAR_AMP_MIDDLENODE_HPP

#include "AudioProcessorNode.hpp"
#include "internal_dsp.hpp"

namespace guitar_amp {
    class MiddleNode : public AudioProcessorNode {
    public:
        
        MiddleNode(int id) : AudioProcessorNode(id) { }
        virtual ~MiddleNode();
        
        virtual void showGui()=0;

        virtual void ApplyFX(const float *in, float *out, size_t numFrames)=0; 

    protected: 
        AudioProcessorNode *input;
        AudioProcessorNode *output;
    };
}

#endif