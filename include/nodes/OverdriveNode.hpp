#ifndef GUITAR_AMP_OVERDRIVENODE_HPP
#define GUITAR_AMP_OVERDRIVENODE_HPP

#include "MiddleNode.hpp"
#include "internal_dsp.hpp"
#include "state.hpp"

namespace guitar_amp {
    class OverdriveNode : public MiddleNode {
    
    public:
        
        OverdriveNode(int id, const AudioInfo current_audio_info);
        ~OverdriveNode();
        
        void showGui();
        void ApplyFX(const float *in, float *out, size_t numFrames, AudioInfo info); 

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

        int clipping_algorithm = 0;

        
        ma_lpf2 lpf;
        ma_hpf2 hpf;


        ma_lpf2 lpf_not_oversampled;
        ma_hpf2 hpf_not_oversampled;

        ma_lpf2 downsample_lpf;

        ma_lpf2_config lpf_config;
        ma_lpf2_config lpf_config_not_oversampled;
        ma_hpf2_config hpf_config;
        ma_hpf2_config hpf_config_not_oversampled;

        ma_resampler upscaler;
        ma_resampler downscaler;

        float *buf_upscale;

    };
}

#endif