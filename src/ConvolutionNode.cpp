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

        ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(ImColor(209,192,8)));
        ImGui::Text("Warning: Experimental convolution support. Audio levels are not guaranteed to be safe.");
        ImGui::PopStyleColor();
        
        ImGui::Checkbox("Enable", &this->enabled);

        if (ImGui::Button("Change Impulse Response")) {
            ImGui::OpenPopup("IR File Explorer");
        }

        if(this->file_browser.showFileDialog("IR File Explorer", imgui_addons::ImGuiFileBrowser::DialogMode::OPEN, ImVec2(100,50), ".wav")) {
            std::thread t(&ConvolutionNode::loadIRFile, this, file_browser.selected_path);
            t.detach(); // Don't hold up the rest of the UI        
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

void ConvolutionNode::ApplyFX(const float *in, float *out, size_t numFrames) { 
    if (this->enabled) {
        this->impulseLock.lock();
        this->convolver.process(in, out, numFrames);
        this->impulseLock.unlock();
    } else {
        memcpy(out,in,numFrames*sizeof(float));
    }
}

bool ConvolutionNode::loadIRFile(const std::string &path) {
    ma_decoder_config decoder_cfg = ma_decoder_config_init(ma_format_f32, 1, device.sampleRate);
    ma_decoder_uninit(&(this->file_reader));
    if (ma_decoder_init_file_wav(path.c_str(), &decoder_cfg, &(this->file_reader)) != MA_SUCCESS) {
        std::cerr << "Unable to open file " << path << "\n";
        return false;
    } else {
        ma_uint64 numFrames;

        if (ma_decoder_get_available_frames(&(this->file_reader), &numFrames) != MA_SUCCESS) {
            std::cerr << "Decoder error in ConvolutionNode::loadIRFile\n";
            return false;
        }

        if (numFrames > 0) {
            this->impulseLock.lock();
            this->impulse.resize(numFrames);
            ma_decoder_read_pcm_frames(&(this->file_reader), this->impulse.data(), numFrames);
            this->convolver.reset();
            this->convolver.init(numFrames/4, this->impulse.data(), numFrames);
            this->impulseLock.unlock();
        } else {
            return false;
        }

        return true;

    }
}