#ifndef GUITAR_AMP_AUDIOPROCESSORNODE_HPP
#define GUITAR_AMP_AUDIOPROCESSORNODE_HPP
#include <SFML/Graphics.hpp>
#include "imgui.h"
#include "imgui-SFML.h"
#include "imnodes.h"

#include <map>

namespace guitar_amp {
    
    class AudioProcessorNode {
    public:

        AudioProcessorNode(int id) { this->id = id; }
        virtual ~AudioProcessorNode();
        
        virtual void showGui()=0;

        int getId() { return this->id; }
        void setId(int id) { this->id = id; }
    
    protected:
        int id;
    };

}

#endif