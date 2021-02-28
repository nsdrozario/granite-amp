#include <ConvolutionNode.hpp>
#include <state.hpp>
#include <iostream>
#include <thread>
using namespace guitar_amp;

ConvolutionNode::ConvolutionNode(int id) : MiddleNode(id), conv_filter(kfr::univector<float>(), device.sampleRate) {
    kfr::audio_reader_wav<float> r (kfr::open_file_for_reading("ir.wav"));
    kfr::univector2d<float> ir_channels = r.read_channels();
    kfr::univector<float> mixed_impulse = ir_channels[0];
    for (size_t i = 1; i < ir_channels.size(); i++) {
        mixed_impulse += ir_channels[i];
    }
    this->impulseLock.lock();
    this->impulse = kfr::univector<float>(mixed_impulse);
    this->conv_filter.set_data(this->impulse);
    this->conv_filter.reset();
    this->impulseLock.unlock();
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

        if (ImGui::Button("Reload")) {
            std::thread t (&guitar_amp::ConvolutionNode::loadIRFile, this, "ir.wav");
            t.detach();
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

void ConvolutionNode::ApplyFX(const kfr::univector<float> &in, kfr::univector<float> &out, size_t numFrames) { 
    out = kfr::univector<float>(in);
    this->impulseLock.lock();
    conv_filter.apply(out);
    this->impulseLock.unlock();
}

bool ConvolutionNode::loadIRFile(std::string path) {
    
    kfr::audio_reader_wav<float> fRead (kfr::open_file_for_reading("ir.wav"));
    kfr::univector2d<float> raw_audio = fRead.read_channels();
    kfr::univector<float> mixed_impulse = raw_audio[0];

    if (fRead.format().samplerate != device.sampleRate) {
        // do sample rate conversion
        std::cout << "bro the sample rate aint the same\n";
        std::cout << "file: " << fRead.format().samplerate << "\n";
        std::cout << "device: " << device.sampleRate << "\n";
    }

    if (fRead.format().channels != 1) {
        // mix down channels, we don't process in stereo yet

        for (size_t i = 1; i < raw_audio.size(); i++) {
            mixed_impulse += raw_audio[i];
        }

    } else {
        mixed_impulse = raw_audio[0];
    }

    this->impulseLock.lock();
    this->impulse = mixed_impulse;
    this->conv_filter.set_data(this->impulse);
    this->conv_filter.reset();
    this->impulseLock.unlock();
    return true;
    
}