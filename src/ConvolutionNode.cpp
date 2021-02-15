#include <ConvolutionNode.hpp>
#include <state.hpp>
using namespace guitar_amp;

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
    imnodes::EndNode();

    ImGui::PopItemWidth();
    imnodes::PopColorStyle();
    imnodes::PopColorStyle();
}

void ConvolutionNode::ApplyFX(const kfr::univector<float> &in, kfr::univector<float> &out, size_t numFrames) { 

}

bool ConvolutionNode::loadIRFile(std::string path) {
    
    kfr::audio_reader_wav<float> fRead (kfr::open_file_for_reading(path));
    kfr::univector2d<float> raw_audio = fRead.read_channels();
    kfr::univector<float> mixed_impulse;

    if (fRead.format().samplerate != device.sampleRate) {
        // do sample rate conversion
    }

    if (fRead.format().channels != 1) {
        // mix down channels, we don't process in stereo yet

        for (auto t : raw_audio) {
            mixed_impulse += t;
        }

    } else {
        mixed_impulse = raw_audio[0];
    }

    this->impulseLock.lock();
    this->impulse = mixed_impulse;
    this->impulseLock.unlock();
    return true;
    
}