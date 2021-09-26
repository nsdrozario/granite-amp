#ifndef GUITAR_AMP_OVERDRIVENODE_HPP
#define GUITAR_AMP_OVERDRIVENODE_HPP

#include "MiddleNode.hpp"
#include "internal_dsp.hpp"
#include "state.hpp"
#include <mindsp/filter.hpp>

namespace guitar_amp {
    class OverdriveNode : public MiddleNode {
    
    public:
        
        OverdriveNode(int id, const AudioInfo current_audio_info);
        OverdriveNode(int id, const AudioInfo current_audio_info, const sol::table &init_table);
        virtual ~OverdriveNode();
        
        void showGui();
        void ApplyFX(const float *in, float *out, size_t numFrames, AudioInfo info); 

        void luaInit(const sol::table &init_table);

        enum ClippingAlgorithm {
            minmax=0,
            tanh=1
        };

    protected: 
    
        AudioProcessorNode *input;
        AudioProcessorNode *output;
    
    private:

        float lpf_cutoff = 12000.0f;
        float hpf_cutoff = 300.0f;
        float gain = 0.0f;
        float output_volume = -35.0f;

        int clipping_algorithm = 1;

        mindsp::filter::biquad_filter low_pass;
        mindsp::filter::biquad_filter high_pass;
        ma_resampler upscaler;
        ma_resampler downscaler;

        float *buf_upscale;

    };
}

#endif