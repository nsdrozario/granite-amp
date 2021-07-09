#ifndef GUITAR_AMP_MIDDLENODE_HPP
#define GUITAR_AMP_MIDDLENODE_HPP

#include "AudioProcessorNode.hpp"
#include "internal_dsp.hpp"

#include <AudioInfo.hpp>

namespace guitar_amp {
    class MiddleNode : public AudioProcessorNode {
    public:
        
        MiddleNode(int id, const AudioInfo current_audio_info);
        virtual ~MiddleNode();
        
        virtual void showGui()=0;

        virtual void ApplyFX(const float *in, float *out, size_t numFrames, guitar_amp::AudioInfo info)=0; 

    protected: 
        AudioInfo internal_info; // in case the node depends on the audio device state
    };
}

#endif