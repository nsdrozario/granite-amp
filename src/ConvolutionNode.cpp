#include <ConvolutionNode.hpp>
#include <state.hpp>
#include <iostream>
#include "internal_dsp.hpp"
#include <thread>
using namespace guitar_amp;

ConvolutionNode::ConvolutionNode(int id, const AudioInfo current_audio_info) : MiddleNode(id, current_audio_info) { 
    
    ma_decoder_config decoder_cfg = ma_decoder_config_init(ma_format_f32, 1, device.sampleRate);

    if (ma_decoder_init_file_wav("ir.wav", &decoder_cfg, &(this->file_reader)) != MA_SUCCESS) {
        
        std::cerr << "Could not open file ir.wav\n";
    
    } else {
        
        ma_uint64 numFrames;

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

        imnodes::PushAttributeFlag(imnodes::AttributeFlags::AttributeFlags_EnableLinkDetachWithDragClick);
        imnodes::BeginInputAttribute(this->id+1);
        imnodes::EndInputAttribute();
        imnodes::BeginOutputAttribute(this->id+3);
        imnodes::EndOutputAttribute();
        imnodes::PopAttributeFlag();

        ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(ImColor(255, 255, 102)));
        ImGui::Text("Some impulse responses may be much louder than others.\nPlease change impulses in bypass mode,\nand then disable bypass mode, set your gain to -144dB,\nand slowly bring up the gain to a desired degree.");
        ImGui::PopStyleColor();

        ImGui::DragFloat("Gain (not applied in bypass mode)", &(this->gain), 1.0f, -144.0f, 0.0f, "%.3f dB");

        ImGui::Checkbox("Bypass", &this->bypass);


        if (ImGui::Button("Change Impulse Response")) {
            ImGui::OpenPopup("IR File Explorer");
        }

        if(this->file_browser.showFileDialog("IR File Explorer", imgui_addons::ImGuiFileBrowser::DialogMode::OPEN, ImVec2(100,50), ".wav")) {
            std::thread t(&ConvolutionNode::loadIRFile, this, file_browser.selected_path);
            t.detach(); // Don't hold up the rest of the UI        
        }

    imnodes::EndNode();

    ImGui::PopItemWidth();
    imnodes::PopColorStyle();
    imnodes::PopColorStyle();
}

void ConvolutionNode::ApplyFX(const float *in, float *out, size_t numFrames, AudioInfo info) { 
    float real_gain = dsp::dbfs_to_f32(this->gain);
    if (!this->bypass) {
        this->impulseLock.lock();
        this->convolver.process(in, out, numFrames);
        this->impulseLock.unlock();
        for (size_t i = 0; i < numFrames; i++) {
            out[i] *= real_gain;
        }
    } else {
        memcpy(out,in,numFrames*sizeof(float));
    }
}

void ConvolutionNode::loadIRFile(const std::string &path) {
    ma_decoder_config decoder_cfg = ma_decoder_config_init(ma_format_f32, 1, device.sampleRate);
    if (ma_decoder_init_file_wav(path.c_str(), &decoder_cfg, &(this->file_reader)) != MA_SUCCESS) {
        std::cerr << "Unable to open file " << path << "\n";
        return;
    } else {
        ma_uint64 numFrames;

        if (ma_decoder_get_available_frames(&(this->file_reader), &numFrames) != MA_SUCCESS) {
            std::cerr << "Decoder error in ConvolutionNode::loadIRFile\n";
            return;
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