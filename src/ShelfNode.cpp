#include <ShelfNode.hpp>
using namespace guitar_amp;


ShelfConfigBuilder::ShelfConfigBuilder(float g, float s, float f) {
    this->gain = g;
    this->slope = s;
    this->frequency = f;
}

ma_hishelf2_config ShelfConfigBuilder::hishelf_cfg() {
    return ma_hishelf2_config_init(
        ma_format_f32, 
        1, 
        (device.sampleRate) ? device.sampleRate : 48000,
        this->gain,
        this->slope,
        this->frequency
    );
}

ma_loshelf2_config ShelfConfigBuilder::loshelf_cfg() {
    return ma_loshelf2_config_init(
        ma_format_f32, 
        1, 
        (device.sampleRate) ? device.sampleRate : 48000,
        this->gain,
        this->slope,
        this->frequency
    );
}

ShelfNode::ShelfNode(int id, const AudioInfo current_audio_info) : MiddleNode(id, current_audio_info) {

    this->high_shelf_cfg = ma_hishelf2_config_init(
        ma_format_f32, 
        1, 
        (device.sampleRate) ? device.sampleRate : 48000,
        0.0, 
        1.0f, 
        10000
    );

    this->low_shelf_cfg = ma_loshelf2_config_init(
        ma_format_f32,
        1,
        (device.sampleRate) ? device.sampleRate : 48000,
        0.0,
        1.0f,
        200.0
    );

    ma_hishelf2_init(&(this->high_shelf_cfg), &(this->high_shelf));
    ma_loshelf2_init(&(this->low_shelf_cfg), &(this->low_shelf));

}

ShelfNode::~ShelfNode() { }

void ShelfNode::showGui() { 

    ImGui::PushItemWidth(100);
    ImNodes::PushColorStyle(ImNodesCol_TitleBar, IM_COL32(80,80,80, 100));
    ImNodes::PushColorStyle(ImNodesCol_TitleBarSelected, IM_COL32(80,80,80, 255));
    ImNodes::PushColorStyle(ImNodesCol_TitleBarHovered, IM_COL32(80,80,80, 255));
    
    ImNodes::BeginNode(id);
        ImNodes::BeginNodeTitleBar();
            ImGui::TextUnformatted("Shelf Filter");
        ImNodes::EndNodeTitleBar();
        
        ImNodes::PushAttributeFlag(ImNodesAttributeFlags_EnableLinkDetachWithDragClick);
        ImNodes::BeginInputAttribute(this->id+1);
        ImNodes::EndInputAttribute();

        ImNodes::BeginOutputAttribute(this->id+3);
        ImNodes::EndOutputAttribute();
        ImNodes::PopAttributeFlag();

        ImGui::Combo("Filter type", &(this->shelf_type), "Low shelf\0High shelf\0");

        if (this->shelf_type == 0) {
            // Low shelf
            bool changed = false;
            if (ImGui::InputDouble("Gain (dB)", &(this->low_shelf_cfg.gainDB), 1.0, 10.0)) {
                changed = true;
            }
            if (ImGui::InputDouble("Slope", &(this->low_shelf_cfg.shelfSlope), 0.1, 0.5)) {
                changed = true;
            }
            if (ImGui::InputDouble("Frequnecy (Hz)", &(this->low_shelf_cfg.frequency), 10.0, 100.0)) {
                changed = true;
            }
            if (changed) {
                ma_loshelf2_reinit(&(this->low_shelf_cfg), &(this->low_shelf));
            }
        } else {
            // High shelf
            bool changed = false;
            if (ImGui::InputDouble("Gain (dB)", &(this->high_shelf_cfg.gainDB), 1.0, 10.0)) {
                changed = true;
            }
            if (ImGui::InputDouble("Slope", &(this->high_shelf_cfg.shelfSlope), 0.1, 0.5)) {
                changed = true;
            }
            if (ImGui::InputDouble("Frequnecy (Hz)", &(this->high_shelf_cfg.frequency), 10.0, 100.0)) {
                changed = true;
            }
            if (changed) {
                ma_hishelf2_reinit(&(this->high_shelf_cfg), &(this->high_shelf));
            }
        }

    ImNodes::EndNode();

    ImGui::PopItemWidth();
    ImNodes::PopColorStyle();
    ImNodes::PopColorStyle();

}

void ShelfNode::ApplyFX(const float *in, float *out, size_t numFrames, AudioInfo info) {
    if (device.sampleRate != this->low_shelf_cfg.sampleRate || device.sampleRate != this->high_shelf_cfg.sampleRate) {
        this->low_shelf_cfg.sampleRate = device.sampleRate;
        this->high_shelf_cfg.sampleRate = device.sampleRate;
        if (this->low_shelf_cfg.shelfSlope <= 0.0) {
            this->low_shelf_cfg.shelfSlope = 0.01;
        }
        if (this->high_shelf_cfg.shelfSlope <= 0.0) {
            this->high_shelf_cfg.shelfSlope = 0.01;
        }
        ma_loshelf2_reinit(&(this->low_shelf_cfg), &(this->low_shelf));
        ma_hishelf2_reinit(&(this->high_shelf_cfg), &(this->high_shelf));
    }
    if (this->shelf_type == 0) {
        ma_loshelf2_process_pcm_frames(&(this->low_shelf), out, in, numFrames);
    } else if (this->shelf_type == 1) {
        ma_hishelf2_process_pcm_frames(&(this->high_shelf), out, in, numFrames);
    }
}