#include <OverdriveNode.hpp>
#include <internal_dsp.hpp>
#include <iostream>

using namespace guitar_amp;

OverdriveNode::OverdriveNode(int id) : MiddleNode(id) {

    this->lpf_config = ma_lpf2_config_init(ma_format_f32, 1, device.sampleRate, this->lpf_cutoff, 0.9f);
    this->hpf_config = ma_hpf2_config_init(ma_format_f32, 1, device.sampleRate, this->hpf_cutoff, 0.9f);

    if (ma_lpf2_init(&lpf_config, &this->lpf) != MA_SUCCESS) {
        std::cout << "lpf failed\n";
    }

    if (ma_hpf2_init(&hpf_config, &this->hpf) != MA_SUCCESS) {
        std::cout << "hpf failed\n";
    }

}

OverdriveNode::~OverdriveNode() {

    

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

        ImGui::DragFloat("Gain", &(this->gain), 1.0, -144, 20, "%.3f dB");
        ImGui::DragFloat("Output Volume", &(this->output_volume), 0.01, -144, 0, "%.3f dB");
        
        if(ImGui::DragFloat("Low pass frequency", &(this->lpf_cutoff), 1, 0, 21000, "%.3f")) {
            this->lpf_config.cutoffFrequency = this->lpf_cutoff;
            ma_lpf2_reinit(&(this->lpf_config),  &(this->lpf));
        }
        
        if(ImGui::DragFloat("High pass frequency", &(this->hpf_cutoff), 1, 0, 21000, "%.3f")) {
            this->hpf_config.cutoffFrequency = this->hpf_cutoff;
            ma_hpf2_reinit(&(this->hpf_config), &(this->hpf));
        }

        // Will be functional after implementing tanh clipping
        // ImGui::Combo("Clipping algorithm", &(this->clipping_algorithm), "minmax\0tanh");
        
        imnodes::BeginOutputAttribute(this->id+3);
        imnodes::EndOutputAttribute();
        imnodes::PopAttributeFlag();
    imnodes::EndNode();

    ImGui::PopItemWidth();
    imnodes::PopColorStyle();
    imnodes::PopColorStyle();

}

void OverdriveNode::ApplyFX(const float *in, float *out, size_t numFrames) {

    memcpy(out, in, sizeof(float) * numFrames);
    ma_hpf2_process_pcm_frames(&this->hpf, out, out, numFrames);

    dsp::hardclip_minmax(out, out, dsp::dbfs_to_f32(this->gain), dsp::dbfs_to_f32(this->output_volume), numFrames);

    ma_lpf2_process_pcm_frames(&this->lpf, out, out, numFrames);

}