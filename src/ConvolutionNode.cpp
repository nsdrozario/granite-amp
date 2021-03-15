#include <ConvolutionNode.hpp>
#include <state.hpp>
#include <iostream>
#include <thread>
using namespace guitar_amp;

ConvolutionNode::ConvolutionNode(int id) : MiddleNode(id) { 
    
    ma_decoder_config decoder_cfg = ma_decoder_config_init(ma_format_f32, 1, device.sampleRate);
    if (ma_decoder_init_file_wav("ir.wav", &decoder_cfg, &(this->file_reader)) != MA_SUCCESS) {
        std::cerr << "Could not open file ir.wav\n";
    }

}

ConvolutionNode::~ConvolutionNode() {
    ma_decoder_uninit(&(this->file_reader));
    this->convolver.reset();    
}

void ConvolutionNode::showGui() {
    ImGui::PushItemWidth(100);
    imnodes::PushColorStyle(imnodes::ColorStyle_TitleBar, IM_COL32(170,110,220, 100));
    imnodes::PushColorStyle(imnodes::ColorStyle_TitleBarSelected, IM_COL32(170,110,220, 255));
    imnodes::PushColorStyle(imnodes::ColorStyle_TitleBarHovered, IM_COL32(170,110,220, 255));
    
    imnodes::BeginNode(id);
        
        imnodes::BeginNodeTitleBar();
            ImGui::TextUnformatted("Convolution IR");
        imnodes::EndNodeTitleBar();

        ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(ImColor(209,192,8)));
        ImGui::Text("Warning: Convolution is not implemented in this build. This node will merely output its input.");
        ImGui::PopStyleColor();
        
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

void ConvolutionNode::ApplyFX(const float *in, float *out, size_t numFrames) { 
    memcpy(out, in, numFrames * sizeof(float)); // assuming mono output
}