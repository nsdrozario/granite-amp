#include <ConvolutionNode.hpp>
#include <state.hpp>
#include <iostream>
#include "internal_dsp.hpp"
#include <thread>
#include <imknob.hpp>

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
        
        ma_decoder_uninit(&file_reader);
    
    }

}

ConvolutionNode::~ConvolutionNode() {
    // decoder should already be released at this point
    this->convolver.reset();    
}

void ConvolutionNode::showGui() {

    imnodes::PushColorStyle(imnodes::ColorStyle_TitleBar, IM_COL32(170,110,220, 100));
    imnodes::PushColorStyle(imnodes::ColorStyle_TitleBarSelected, IM_COL32(170,110,220, 255));
    imnodes::PushColorStyle(imnodes::ColorStyle_TitleBarHovered, IM_COL32(170,110,220, 255));
    
    imnodes::BeginNode(id);

        imnodes::BeginNodeTitleBar();
            ImGui::TextUnformatted("Convolution Reverb");
        imnodes::EndNodeTitleBar();

        imnodes::PushAttributeFlag(imnodes::AttributeFlags::AttributeFlags_EnableLinkDetachWithDragClick);
        imnodes::BeginInputAttribute(this->id+1);
        imnodes::EndInputAttribute();
        imnodes::BeginOutputAttribute(this->id+3);
        imnodes::EndOutputAttribute();
        imnodes::PopAttributeFlag();

        ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(ImColor(255, 255, 102)));
        ImGui::Text("Keep the gain low and slowly raise the gain\nwhen loading in new impulse responses,\nthey could be louder than expected.");
        ImGui::PopStyleColor();

        if (advancedMode) {
            ImGui::DragFloat("Gain", &(this->gain), 1.0f, -144.0f, 0.0f, "%.3f dB");
        } else {
            ImKnob::Knob("Gain", &gain, 1.0f, -144.0f, 0.0f, "%.0f dB", 24.0f, ImVec4(0.1f,0.1f,0.1f,1.0f), ImVec4(0.15f,0.15f,0.15f,1.0f));
        }

        ImGui::Checkbox("Bypass", &this->bypass);


        if (ImGui::Button("Change Impulse Response")) {
            ImGui::OpenPopup("IR File Explorer");
        }

        if(this->file_browser.showFileDialog("IR File Explorer", imgui_addons::ImGuiFileBrowser::DialogMode::OPEN, ImVec2(100,50), ".wav")) {
            std::thread t(&ConvolutionNode::loadIRFile, this, file_browser.selected_path);
            t.detach(); // Don't hold up the rest of the UI        
        }

    imnodes::EndNode();

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
        for (size_t i = 0; i < numFrames; i++) {
            out[i] = in[i] * real_gain;
        }
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
        ma_decoder_uninit(&file_reader);
    }
}