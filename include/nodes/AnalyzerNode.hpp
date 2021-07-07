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
        AnalyzerNode(int id);
        virtual ~AnalyzerNode();    
        virtual void showGui();
        virtual void ApplyFX(const float *in, float *out, size_t numFrames, const AudioInfo &info);

        void reinit();
    
    private:
        bool showing_spectrum;
        bool accept_warning;
        size_t fft_size;
        std::vector<float> signal;
        std::vector<float> real;
        std::vector<float> imaginary;
        std::vector<float> output;
        std::vector<float> freqs;
        std::mutex output_lock;
        audiofft::AudioFFT internal_fft;
    };
}

#endif