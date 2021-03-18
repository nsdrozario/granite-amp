#include <ConvolutionNode.hpp>
#include <state.hpp>
#include <iostream>
#include <thread>
using namespace guitar_amp;

ConvolutionNode::ConvolutionNode(int id) : MiddleNode(id) { 
    
    ma_decoder_config decoder_cfg = ma_decoder_config_init(ma_format_f32, 1, device.sampleRate);

    if (ma_decoder_init_file_wav("ir.wav", &decoder_cfg, &(this->file_reader)) != MA_SUCCESS) {
        
        std::cerr << "Could not open file ir.wav\n";
    
    } else {
        
        size_t numFrames;

        if (ma_decoder_get_available_frames(&(this->file_reader), &numFrames) != MA_SUCCESS) {
            std::cerr << "Decoder error in ConvolutionNode constructor\n";
        }

        if (numFrames > 0) {
            this->impulseLock.lock();
            this->impulse.resize(numFrames);
            ma_decoder_read_pcm_frames(&(this->file_reader), this->impulse.data(), numFrames);
            this->convolver.init(numFrames/4, this->impulse.data(), numFrames);
            this->impulseLock.unlock();
        }

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
        ImGui::Text("Warning: Experimental convolution support. Audio levels are not guaranteed to be safe.");
        ImGui::PopStyleColor();
        
        ImGui::Checkbox("Enable", &this->enabled);

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
    if (this->enabled) {
        this->impulseLock.lock();
        this->convolver.process(in, out, numFrames);
        this->impulseLock.unlock();
    }
}