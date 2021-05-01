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

ShelfNode::ShelfNode(int id) : MiddleNode(id) {

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
    imnodes::PushColorStyle(imnodes::ColorStyle_TitleBar, IM_COL32(80,80,80, 100));
    imnodes::PushColorStyle(imnodes::ColorStyle_TitleBarSelected, IM_COL32(80,80,80, 255));
    imnodes::PushColorStyle(imnodes::ColorStyle_TitleBarHovered, IM_COL32(80,80,80, 255));
    
    imnodes::BeginNode(id);
        imnodes::BeginNodeTitleBar();
            ImGui::TextUnformatted("Shelf Filter");
        imnodes::EndNodeTitleBar();

        ImGui::Combo("Filter type", &(this->shelf_type), "Low shelf\0High shelf\0");

        if (this->shelf_type == 0) {
            // Low shelf
            bool changed = false;
            if (ImGui::InputDouble("Gain (dB)", &(this->low_shelf_cfg.gainDB), 1.0, 10.0)) {
                changed = true;
            }
            if (ImGui::InputDouble("Slope", &(this->low_shelf_cfg.shelfSlope), 1.0, 10.0)) {
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
            if (ImGui::InputDouble("Slope", &(this->high_shelf_cfg.shelfSlope), 1.0, 10.0)) {
                changed = true;
            }
            if (ImGui::InputDouble("Frequnecy (Hz)", &(this->high_shelf_cfg.frequency), 10.0, 100.0)) {
                changed = true;
            }
            if (changed) {
                ma_hishelf2_reinit(&(this->high_shelf_cfg), &(this->high_shelf));
            }
        }

        imnodes::PushAttributeFlag(imnodes::AttributeFlags::AttributeFlags_EnableLinkDetachWithDragClick);
        imnodes::BeginInputAttribute(this->id+1);
        imnodes::EndInputAttribute();

        imnodes::BeginOutputAttribute(this->id+3);
        imnodes::EndOutputAttribute();
        imnodes::PopAttributeFlag();
    imnodes::EndNode();

    ImGui::PopItemWidth();
    imnodes::PopColorStyle();
    imnodes::PopColorStyle();

}

void ShelfNode::ApplyFX(const float *in, float *out, size_t numFrames) {
    if (device.sampleRate != this->low_shelf_cfg.sampleRate || device.sampleRate != this->high_shelf_cfg.sampleRate) {
        this->low_shelf_cfg.sampleRate = device.sampleRate;
        this->high_shelf_cfg.sampleRate = device.sampleRate;
        ma_loshelf2_reinit(&(this->low_shelf_cfg), &(this->low_shelf));
        ma_hishelf2_reinit(&(this->high_shelf_cfg), &(this->high_shelf));
    }
    if (this->shelf_type == 0) {
        ma_loshelf2_process_pcm_frames(&(this->low_shelf), out, in, numFrames);
    } else if (this->shelf_type == 1) {
        ma_hishelf2_process_pcm_frames(&(this->high_shelf), out, in, numFrames);
    }
}