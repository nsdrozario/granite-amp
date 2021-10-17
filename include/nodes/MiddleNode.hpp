#ifndef GUITAR_AMP_MIDDLENODE_HPP
#define GUITAR_AMP_MIDDLENODE_HPP

#include "AudioProcessorNode.hpp"
#include "internal_dsp.hpp"

#include <AudioInfo.hpp>
#include <imgui.h>
#include <sol/sol.hpp>

const ImVec4 COLOR_KNOB_DARK (0.1f,0.1f,0.1f,1.0f);
const ImVec4 COLOR_KNOB_DARK_SELECTED  (0.15f,0.15f,0.15f,1.0f);

namespace guitar_amp {
    class MiddleNode : public AudioProcessorNode {
    public:
        
        MiddleNode(int id, const AudioInfo current_audio_info);
        MiddleNode(int id, const AudioInfo current_audio_info, const sol::table &init_table);
        virtual ~MiddleNode();
        
        virtual void showGui()=0;

        virtual void ApplyFX(const float *in, float *out, size_t numFrames, guitar_amp::AudioInfo info)=0;

        virtual void luaInit(const sol::table &init_table) = 0;

        virtual sol::table serializeLua() = 0;

    protected: 
        AudioInfo internal_info; // in case the node depends on the audio device state
    };
}

#endif