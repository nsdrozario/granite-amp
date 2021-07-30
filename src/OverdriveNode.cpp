#include <OverdriveNode.hpp>
#include <internal_dsp.hpp>
#include <iostream>
#include <imknob.hpp>

using namespace guitar_amp;

OverdriveNode::OverdriveNode(int id, const AudioInfo current_audio_info) : MiddleNode(id, current_audio_info) {

    this->lpf_config = ma_lpf2_config_init(ma_format_f32, 1, current_audio_info.sample_rate*4, this->lpf_cutoff, 0.9f);
    this->hpf_config = ma_hpf2_config_init(ma_format_f32, 1, current_audio_info.sample_rate*4, this->hpf_cutoff, 0.9f);

    lpf_config_not_oversampled = ma_lpf2_config_init(
        ma_format_f32,
        1,
        current_audio_info.sample_rate,
        this->lpf_cutoff,
        0.9f
    );
    
    hpf_config_not_oversampled = ma_hpf2_config_init(
        ma_format_f32,
        1,
        current_audio_info.sample_rate,
        this->hpf_cutoff,
        0.9f
    );

    if (ma_lpf2_init(&lpf_config, &this->lpf) != MA_SUCCESS) {
        std::cout << "lpf failed\n";
    }

    if (ma_lpf2_init(&lpf_config_not_oversampled, &lpf_not_oversampled) != MA_SUCCESS) {
        std::cout << "lpf failed\n";
    }

    if (ma_hpf2_init(&hpf_config, &this->hpf) != MA_SUCCESS) {
        std::cout << "hpf failed\n";
    }

    if (ma_hpf2_init(&hpf_config_not_oversampled, &hpf_not_oversampled) != MA_SUCCESS) {
        std::cout << "hpf failed\n";
    }

    ma_resampler_config resampler_cfg_up = ma_resampler_config_init(
        ma_format_f32, 
        1, 
        current_audio_info.sample_rate, 
        current_audio_info.sample_rate * 4,  
        ma_resample_algorithm_linear
    );
    
    ma_resampler_config resampler_cfg_down = ma_resampler_config_init(
        ma_format_f32,
        1,
        current_audio_info.sample_rate * 4,
        current_audio_info.sample_rate,
        ma_resample_algorithm_linear
    );

    ma_lpf2_config downsample_lpf_cfg = ma_lpf2_config_init(
        ma_format_f32,
        1,
        current_audio_info.sample_rate,
        static_cast<double>(current_audio_info.sample_rate) * 0.5,
        1.0
    );

    ma_resampler_init(&resampler_cfg_up, &upscaler);
    ma_resampler_init(&resampler_cfg_down, &downscaler);
    ma_lpf2_init(&downsample_lpf_cfg, &downsample_lpf);

    buf_upscale = new float[current_audio_info.period_length * 4];
}

OverdriveNode::~OverdriveNode() {

    delete[] buf_upscale;

}

void OverdriveNode::showGui() {

    ImGui::PushItemWidth(100);
    imnodes::PushColorStyle(imnodes::ColorStyle_TitleBar, IM_COL32(201, 4, 126, 100));
    imnodes::PushColorStyle(imnodes::ColorStyle_TitleBarSelected, IM_COL32(201, 4, 126, 255));
    imnodes::PushColorStyle(imnodes::ColorStyle_TitleBarHovered, IM_COL32(201, 4, 126, 255));

    imnodes::BeginNode(this->id);

        imnodes::BeginNodeTitleBar();
            ImGui::TextUnformatted("Overdrive");
        imnodes::EndNodeTitleBar();
        
        imnodes::PushAttributeFlag(imnodes::AttributeFlags::AttributeFlags_EnableLinkDetachWithDragClick);
        imnodes::BeginInputAttribute(this->id+1);
        imnodes::EndInputAttribute();
        imnodes::BeginOutputAttribute(this->id+3);
        imnodes::EndOutputAttribute();
        imnodes::PopAttributeFlag();

        if (advancedMode) {
            ImGui::DragFloat("Gain", &(this->gain), 1.0, -144, 70.0f, "%.3f dB");
            
            // with how high the gain can go the -8dB limit is for the safety of the user
            ImGui::DragFloat("Output Volume", &(this->output_volume), 0.01, -144, -8.0f, "%.3f dB");
            
            if(ImGui::DragFloat("Low pass frequency", &(this->lpf_cutoff), 1, 0, 21000, "%.3f")) {
                lpf_config.cutoffFrequency = lpf_cutoff;
                lpf_config_not_oversampled.cutoffFrequency = lpf_cutoff;
                ma_lpf2_reinit(&lpf_config,  &lpf);
                ma_lpf2_reinit(&lpf_config_not_oversampled, &lpf_not_oversampled);
            }
            
            if(ImGui::DragFloat("High pass frequency", &(this->hpf_cutoff), 1, 0, 21000, "%.3f Hz")) {
                hpf_config.cutoffFrequency = hpf_cutoff;
                hpf_config_not_oversampled.cutoffFrequency = hpf_cutoff;
                ma_hpf2_reinit(&hpf_config, &hpf);
                ma_hpf2_reinit(&hpf_config_not_oversampled, &hpf_not_oversampled);
            }
        } else {

            ImKnob::Knob("Gain", &gain, 1.0, -144, 70, "%.0f dB", 24.0f, ImVec4(0.1f,0.1f,0.1f,1.0f), ImVec4(0.15f,0.15f,0.15f,1.0f));
            
            ImGui::SameLine();

            if(ImKnob::Knob("Treble", &(this->lpf_cutoff), 1, 1000, 21000, "%.0f Hz", 24.0f, ImVec4(0.1f,0.1f,0.1f,1.0f), ImVec4(0.15f,0.15f,0.15f,1.0f))) {
                lpf_config.cutoffFrequency = lpf_cutoff;
                lpf_config_not_oversampled.cutoffFrequency = lpf_cutoff;
                ma_lpf2_reinit(&lpf_config,  &lpf);
                ma_lpf2_reinit(&lpf_config_not_oversampled, &lpf_not_oversampled);
            }
            
            ImGui::SameLine();

            if(ImKnob::Knob("Bass", &(this->hpf_cutoff), 1, 10, 1000, "%.0f Hz", 24.0f, ImVec4(0.1f,0.1f,0.1f,1.0f), ImVec4(0.15f,0.15f,0.15f,1.0f))) {
                hpf_config.cutoffFrequency = hpf_cutoff;
                hpf_config_not_oversampled.cutoffFrequency = hpf_cutoff;
                ma_hpf2_reinit(&hpf_config, &hpf);
                ma_hpf2_reinit(&hpf_config_not_oversampled, &hpf_not_oversampled);
            }

        }

    imnodes::EndNode();

    ImGui::PopItemWidth();
    imnodes::PopColorStyle();
    imnodes::PopColorStyle();

}

void OverdriveNode::ApplyFX(const float *in, float *out, size_t numFrames, AudioInfo info) {

    if (info != internal_info) {
        // need to update everything
    }

    if (oversamplingEnabled) {

        if (buf_upscale == nullptr) { // this really shouldn't happen
            buf_upscale = new float[numFrames * 4];
        }

        ma_uint64 frames_to_process = numFrames;
        ma_uint64 frames_expected = numFrames * 4;

        // hopefully this doesn't take too long to run
        // resample to 4x the sampling rate
        do {
            ma_result resample_1st_result = ma_resampler_process_pcm_frames(
                &upscaler, 
                in, 
                &frames_to_process, 
                buf_upscale, 
                &frames_expected
            );

            if (resample_1st_result != MA_SUCCESS) {
                // something went wrong
            }
        } while (
            (numFrames - frames_to_process != 0) && ((numFrames * 4) - frames_expected) != 0
        );

        // get rid of the bass frequencies
        ma_hpf2_process_pcm_frames(&this->hpf, buf_upscale, buf_upscale, numFrames * 4);

        // apply a transfer function for saturation and clipping
        switch(this->clipping_algorithm) {
            case 0:
                dsp::hardclip_minmax(buf_upscale, buf_upscale, this->gain, this->output_volume, numFrames * 4);
                break;
            case 1:
                dsp::clip_tanh(buf_upscale, buf_upscale, this->gain, this->output_volume, numFrames * 4);
                break;
            case 2:
                dsp::clip_sin(buf_upscale, buf_upscale, this->gain, this->output_volume, numFrames * 4);
                break;
        }

        // low pass at the nyquist frequency 
        ma_lpf2_process_pcm_frames(&downsample_lpf, buf_upscale, buf_upscale, numFrames * 4);

        ma_uint64 frames_to_process_downscale = numFrames * 4;
        ma_uint64 frames_expected_downscale = numFrames;
        // resample to the playback sampling rate
        do {
            ma_result downscale_result = ma_resampler_process_pcm_frames(
            &downscaler, 
            buf_upscale, 
            &frames_to_process_downscale,
            out, 
            &frames_expected_downscale
            );
            if (downscale_result != MA_SUCCESS) {
                
            }
        } while (
            (numFrames * 4) - frames_to_process_downscale != 0 && numFrames - frames_expected_downscale != 0
        );
    } else {
        // if we don't want oversampling, do same thing but with no resampling

        ma_hpf2_process_pcm_frames(&hpf_not_oversampled, out, in, numFrames);

        switch(this->clipping_algorithm) {
            case 0:
                dsp::hardclip_minmax(out, out, this->gain, this->output_volume, numFrames);
                break;
            case 1:
                dsp::clip_tanh(out, out, this->gain, this->output_volume, numFrames);
                break;
            case 2:
                dsp::clip_sin(out, out, this->gain, this->output_volume, numFrames);
                break;
        }

    }
    // get rid of the treble frequencies as desired
    ma_lpf2_process_pcm_frames(&this->lpf, out, out, numFrames);

}