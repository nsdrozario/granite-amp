#include <OverdriveNode.hpp>
#include <internal_dsp.hpp>
#include <iostream>
#include <imknob.hpp>
#include <miniaudio.h>

#define OVERSAMPLIING_FACTOR 8

using namespace guitar_amp;

OverdriveNode::OverdriveNode(int id, const AudioInfo current_audio_info) : MiddleNode(id, current_audio_info) {

    low_pass.set_coefficients(
       mindsp::filter::low_pass_filter(
           lpf_cutoff, 
           (current_audio_info.sample_rate) ? static_cast<float>(current_audio_info.sample_rate) : 48000, 
           1.0f
        )
    );

    high_pass.set_coefficients(
        mindsp::filter::high_pass_filter(
            hpf_cutoff,
            (current_audio_info.sample_rate) ? static_cast<float>(current_audio_info.sample_rate) : 48000,
            1.0f
        )
    );

    ma_resampler_config resampler_cfg_up = ma_resampler_config_init(
        ma_format_f32, 
        1, 
        current_audio_info.sample_rate, 
        current_audio_info.sample_rate * OVERSAMPLIING_FACTOR,  
        ma_resample_algorithm_linear
    );
    
    ma_resampler_config resampler_cfg_down = ma_resampler_config_init(
        ma_format_f32,
        1,
        current_audio_info.sample_rate * OVERSAMPLIING_FACTOR,
        current_audio_info.sample_rate,
        ma_resample_algorithm_linear
    );


    ma_resampler_init(&resampler_cfg_up, &upscaler);
    ma_resampler_init(&resampler_cfg_down, &downscaler);

    buf_upscale = new float[current_audio_info.period_length * OVERSAMPLIING_FACTOR];
}

OverdriveNode::~OverdriveNode() {

    delete[] buf_upscale;

}

OverdriveNode::OverdriveNode(int id, const AudioInfo current_audio_info, const sol::table &init_table) : MiddleNode(id, current_audio_info) {

    /*

        Exmaple config:

        {
            ["HPF"]=12000,
            ["LPF"]=300,
            ["Gain"]=0,
            ["Volume"]=-35
        }

    */

    lpf_cutoff = init_table.get_or("LPF", 12000.0);
    hpf_cutoff = init_table.get_or("HPF", 300.0);
    gain = init_table.get_or("Gain", 0.0);
    output_volume = init_table.get_or("Volume", -35.0);

    low_pass.set_coefficients(
       mindsp::filter::low_pass_filter(
           lpf_cutoff, 
           (current_audio_info.sample_rate) ? static_cast<float>(current_audio_info.sample_rate) : 48000, 
           1.0f
        )
    );

    high_pass.set_coefficients(
        mindsp::filter::high_pass_filter(
            hpf_cutoff,
            (current_audio_info.sample_rate) ? static_cast<float>(current_audio_info.sample_rate) : 48000,
            1.0f
        )
    );

    ma_resampler_config resampler_cfg_up = ma_resampler_config_init(
        ma_format_f32, 
        1, 
        current_audio_info.sample_rate, 
        current_audio_info.sample_rate * OVERSAMPLIING_FACTOR,  
        ma_resample_algorithm_linear
    );
    
    ma_resampler_config resampler_cfg_down = ma_resampler_config_init(
        ma_format_f32,
        1,
        current_audio_info.sample_rate * OVERSAMPLIING_FACTOR,
        current_audio_info.sample_rate,
        ma_resample_algorithm_linear
    );


    ma_resampler_init(&resampler_cfg_up, &upscaler);
    ma_resampler_init(&resampler_cfg_down, &downscaler);

    buf_upscale = new float[current_audio_info.period_length * OVERSAMPLIING_FACTOR];

}

sol::table OverdriveNode::serializeLua() {

    /*

        Exmaple config:

        {
            ["HPF"]=12000,
            ["LPF"]=300,
            ["Gain"]=0,
            ["Volume"]=-35
        }

    */

    sol::table out = sol::table();
    sol::table state = sol::table();

    out.set("type", "Overdrive");

    state.set("HPF", hpf_cutoff);
    state.set("LPF", lpf_cutoff);
    state.set("Gain", gain);
    state.set("Volume", output_volume);

    out.set("state", state);

    return out;

}

void OverdriveNode::luaInit(const sol::table &init_table) {
    lpf_cutoff = init_table.get_or("LPF", 12000.0);
    hpf_cutoff = init_table.get_or("HPF", 300.0);
    gain = init_table.get_or("Gain", 0.0);
    output_volume = init_table.get_or("Volume", -35.0);

    low_pass.set_coefficients(
       mindsp::filter::low_pass_filter(
           lpf_cutoff, 
           (internal_info.sample_rate) ? static_cast<float>(internal_info.sample_rate) : 48000, 
           1.0f
        )
    );

    high_pass.set_coefficients(
        mindsp::filter::high_pass_filter(
            hpf_cutoff,
            (internal_info.sample_rate) ? static_cast<float>(internal_info.sample_rate) : 48000,
            1.0f
        )
    );

    ma_resampler_config resampler_cfg_up = ma_resampler_config_init(
        ma_format_f32, 
        1, 
        internal_info.sample_rate, 
        internal_info.sample_rate * OVERSAMPLIING_FACTOR,  
        ma_resample_algorithm_linear
    );
    
    ma_resampler_config resampler_cfg_down = ma_resampler_config_init(
        ma_format_f32,
        1,
        internal_info.sample_rate * OVERSAMPLIING_FACTOR,
        internal_info.sample_rate,
        ma_resample_algorithm_linear
    );


    ma_resampler_init(&resampler_cfg_up, &upscaler);
    ma_resampler_init(&resampler_cfg_down, &downscaler);

    buf_upscale = new float[internal_info.period_length * OVERSAMPLIING_FACTOR];
}

void OverdriveNode::showGui() {

    ImNodes::PushColorStyle(ImNodesCol_TitleBar, IM_COL32(245, 39, 7, 100));
    ImNodes::PushColorStyle(ImNodesCol_TitleBarSelected, IM_COL32(245, 39, 7, 255));
    ImNodes::PushColorStyle(ImNodesCol_TitleBarHovered, IM_COL32(245, 39, 7, 255));
    if (advancedMode) {
        ImGui::PushItemWidth(150);
    }
    ImNodes::BeginNode(this->id);
        ImNodes::BeginNodeTitleBar();
            ImGui::TextUnformatted("Overdrive");
        ImNodes::EndNodeTitleBar();
        
        ImNodes::PushAttributeFlag(ImNodesAttributeFlags_EnableLinkDetachWithDragClick);
        ImNodes::BeginInputAttribute(this->id+1, ImNodesPinShape_TriangleFilled);
        ImNodes::EndInputAttribute();
        ImNodes::BeginOutputAttribute(this->id+3, ImNodesPinShape_TriangleFilled);
        ImNodes::EndOutputAttribute();
        ImNodes::PopAttributeFlag();

        if (advancedMode) {
            ImGui::DragFloat("Gain", &(this->gain), 1.0, -144, 40.0f, "%.3f dB");
            
            // with how high the gain can go the -8dB limit is for the safety of the user
            ImGui::DragFloat("Output Volume", &(this->output_volume), 0.01, -144, -8.0f, "%.3f dB");
            
            if(ImGui::DragFloat("Low pass frequency", &(this->lpf_cutoff), 1, 0, 21000, "%.3f")) {
                low_pass.set_coefficients(mindsp::filter::low_pass_filter(lpf_cutoff, device.sampleRate ? device.sampleRate : 48000, 1.0f));
            }
            
            if(ImGui::DragFloat("High pass frequency", &(this->hpf_cutoff), 1, 0, 21000, "%.3f Hz")) {  
                high_pass.set_coefficients(mindsp::filter::high_pass_filter(hpf_cutoff, device.sampleRate ? device.sampleRate : 48000, 1.0f));
            }

        } else {
            ImKnob::Knob("Gain", &gain, 1.0, -14, 40, "%.0f dB", 24.0f, COLOR_KNOB_DARK, COLOR_KNOB_DARK_SELECTED);

            ImGui::SameLine();

            ImKnob::Knob("Volume", &output_volume, 1.0, -50, -14, "%.0f dB", 24.0f, COLOR_KNOB_DARK, COLOR_KNOB_DARK_SELECTED);

            if(ImKnob::Knob("Treble", &(this->lpf_cutoff), 1, 1000, 21000, "%.0f Hz", 24.0f, ImVec4(0.1f,0.1f,0.1f,1.0f), ImVec4(0.15f,0.15f,0.15f,1.0f))) {
                low_pass.set_coefficients(mindsp::filter::low_pass_filter(lpf_cutoff, device.sampleRate ? device.sampleRate : 48000, 1.0f));
            }
            
            ImGui::SameLine();

            if(ImKnob::Knob("Bass", &(this->hpf_cutoff), 1, 10, 1000, "%.0f Hz", 24.0f, ImVec4(0.1f,0.1f,0.1f,1.0f), ImVec4(0.15f,0.15f,0.15f,1.0f))) {
                high_pass.set_coefficients(mindsp::filter::high_pass_filter(hpf_cutoff, device.sampleRate ? device.sampleRate : 48000, 1.0f));
            }

        }

    ImNodes::EndNode();
    if (advancedMode) {
        ImGui::PopItemWidth();
    }
    ImNodes::PopColorStyle();
    ImNodes::PopColorStyle();

}

void OverdriveNode::ApplyFX(const float *in, float *out, size_t numFrames, AudioInfo info) {

    if (info != internal_info) {
        
        ma_resampler_uninit(&upscaler);
        ma_resampler_uninit(&downscaler);

        ma_resampler_config upscaler_cfg = ma_resampler_config_init(
            ma_format_f32,
            1,
            info.sample_rate,
            info.sample_rate * OVERSAMPLIING_FACTOR,
            ma_resample_algorithm_linear
        );

        ma_resampler_config downscaler_cfg = ma_resampler_config_init(
            ma_format_f32,
            1,
            info.sample_rate * OVERSAMPLIING_FACTOR,
            info.sample_rate,
            ma_resample_algorithm_linear
        );

        ma_resampler_init(&upscaler_cfg, &upscaler);
        ma_resampler_init(&downscaler_cfg, &downscaler);

        low_pass.set_coefficients(mindsp::filter::low_pass_filter(lpf_cutoff, device.sampleRate ? device.sampleRate : 48000, 1.0f));
        high_pass.set_coefficients(mindsp::filter::high_pass_filter(hpf_cutoff, device.sampleRate ? device.sampleRate : 48000, 1.0f));

        internal_info = info;
    
    }

    high_pass.apply(out, in, numFrames);
    /*
    if (oversamplingEnabled) {

        if (buf_upscale == nullptr) { // this really shouldn't happen
            buf_upscale = new float[numFrames * OVERSAMPLIING_FACTOR];
        }

        ma_uint64 frames_to_process = numFrames;
        ma_uint64 frames_expected = numFrames * OVERSAMPLIING_FACTOR;

        // hopefully this doesn't take too long to run
        do {
            ma_result resample_1st_result = ma_resampler_process_pcm_frames(
                &upscaler, 
                out, 
                &frames_to_process, 
                buf_upscale, 
                &frames_expected
            );

            if (resample_1st_result != MA_SUCCESS) {
                // something went wrong
            }
        } while (
            (numFrames - frames_to_process != 0) && ((numFrames * OVERSAMPLIING_FACTOR) - frames_expected) != 0
        );

        // apply a transfer function for saturation and clipping
        switch(this->clipping_algorithm) {
            case 0:
                dsp::hardclip_minmax(buf_upscale, buf_upscale, this->gain, this->output_volume, numFrames * OVERSAMPLIING_FACTOR);
                break;
            case 1:
                dsp::clip_tanh(buf_upscale, buf_upscale, this->gain, this->output_volume, numFrames * OVERSAMPLIING_FACTOR);
                break;
            case 2:
                dsp::clip_sin(buf_upscale, buf_upscale, this->gain, this->output_volume, numFrames * OVERSAMPLIING_FACTOR);
                break;
        }

        ma_uint64 frames_to_process_downscale = numFrames * OVERSAMPLIING_FACTOR;
        ma_uint64 frames_expected_downscale = numFrames;
        // downsample to the playback sampling rate
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
            (numFrames * OVERSAMPLIING_FACTOR) - frames_to_process_downscale != 0 && numFrames - frames_expected_downscale != 0
        );

    } else {
        */
       /*
        switch(this->clipping_algorithm) {
            case 0:
                dsp::hardclip_minmax(out, out, this->gain, this->output_volume, numFrames);
                break;
            case 1:
            */
                dsp::clip_tanh(out, out, this->gain, this->output_volume, numFrames);
            /*
                break;
            case 2:
                dsp::clip_sin(out, out, this->gain, this->output_volume, numFrames);
                break;
        }
        */
    /*
    }
    */
   
    low_pass.apply(out, out, numFrames);
    
}