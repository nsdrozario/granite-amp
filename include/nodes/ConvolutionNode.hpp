#ifndef GUITAR_AMP_CONVOLUTIONNODE_HPP
#define GUITAR_AMP_CONVOLUTIONNODE_HPP
#define FFTCONVOLVER_DONT_USE_SSE

#include "MiddleNode.hpp"
#include "internal_dsp.hpp"
#include <string>
#include <thread>
#include <mutex>
#include "ImGuiFileBrowser.h"
#include <vector>
#include "FFTConvolver.h"

namespace guitar_amp {
    class ConvolutionNode : public MiddleNode {
    
    public:
        
        ConvolutionNode(int id, const AudioInfo current_audio_info);
        ConvolutionNode(int id, const AudioInfo current_audio_info, const sol::table &init_table);
        virtual ~ConvolutionNode();
        
        void showGui();
        void ApplyFX(const float *in, float *out, size_t numFrames, AudioInfo info); 
        void luaInit(const sol::table &init_table);
        virtual sol::table serializeLua();

    private:

        std::mutex impulseLock;
        std::vector<float> impulse;
        ma_decoder file_reader;
        fftconvolver::FFTConvolver convolver;
        bool bypass = true;
        imgui_addons::ImGuiFileBrowser file_browser;
        float gain = -144.0f;

        void loadIRFile(const std::string &path);

    };
}

#endif