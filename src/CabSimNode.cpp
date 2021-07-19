#include <CabSimNode.hpp>
using namespace guitar_amp;

CabSimNode::CabSimNode(int id, const AudioInfo current_audio_info) : MiddleNode(id, current_audio_info) {

    // set up convolver
    size_t convolver_block_size = (device.capture.internalPeriodSizeInFrames) ? device.capture.internalPeriodSizeInFrames/2 : 256;
    
    last_period_size = 512;
}

CabSimNode::~CabSimNode() { }

void CabSimNode::showGui() {
    ImGui::PushItemWidth(100);
    imnodes::PushColorStyle(imnodes::ColorStyle_TitleBar, IM_COL32(170,110,220, 100));
    imnodes::PushColorStyle(imnodes::ColorStyle_TitleBarSelected, IM_COL32(170,110,220, 255));
    imnodes::PushColorStyle(imnodes::ColorStyle_TitleBarHovered, IM_COL32(170,110,220, 255));
    
    imnodes::BeginNode(id);
        
        imnodes::BeginNodeTitleBar();
            ImGui::TextUnformatted("Algorithmic Cabinet Simulation");
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

void CabSimNode::ApplyFX(const float *in, float *out, size_t numFrames, AudioInfo info) {



    convolver.process(in, out, numFrames);

}