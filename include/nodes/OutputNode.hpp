#ifndef GUITAR_AMP_OUTPUTNODE_HPP
#define GUITAR_AMP_OUTPUTNODE_HPP

#include "AudioProcessorNode.hpp"

namespace guitar_amp {
    class OutputNode : public AudioProcessorNode {
    public:
        OutputNode(int id) : AudioProcessorNode(id) { };
        ~OutputNode();
        
        float getGain();
        void setGain(float g);

        void showGui();

    private:
        float gain = 0.0f;
    };
}

#endif