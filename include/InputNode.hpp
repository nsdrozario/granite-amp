#ifndef GUITAR_AMP_INPUTNODE_HPP
#define GUITAR_AMP_INPUTNODE_HPP

#include "AudioProcessorNode.hpp"

namespace guitar_amp {
    class InputNode : public AudioProcessorNode {
    public:
        InputNode();
        
        float getGain();
        void setGain(float g);

        void showGui();

        AudioProcessorNode *next;

    private:
        float gain = 0.0f;
    };
}

#endif