#ifndef GUITAR_AMP_ANALYZERNODE_HPP
#define GUITAR_AMP_ANALYZERNODE_HPP

#include "state.hpp"
#include "internal_dsp.hpp"
#include "MiddleNode.hpp"
#include "AudioFFT.h"
#include "FFTConvolver.h"
#include <mutex>

namespace guitar_amp {
    class AnalyzerNode : public MiddleNode {
    public:
        AnalyzerNode(int id, const AudioInfo current_audio_info);
        AnalyzerNode(int id, const AudioInfo current_audio_info, const sol::table &init_table);

        virtual ~AnalyzerNode();    
        virtual void showGui();
        virtual void ApplyFX(const float *in, float *out, size_t numFrames, AudioInfo info);

        void luaInit(const sol::table &init_table);
        void reinit();
        virtual sol::table serializeLua();

    private:
        bool showing_spectrum = false;
        bool accept_warning = false;
        size_t fft_size;
        std::vector<float> signal;
        std::vector<float> real;
        std::vector<float> imaginary;
        std::vector<float> output;
        std::vector<float> freqs;
        std::mutex output_lock;
        audiofft::AudioFFT internal_fft;
        bool freqDomain = true;
        std::vector<float> time_domain_data;
        std::vector<float> time_domain_labels;
    };
}

#endif