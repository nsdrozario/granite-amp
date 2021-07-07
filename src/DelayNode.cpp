#include <DelayNode.hpp>
#include <algorithm>
#include <iostream>
using namespace guitar_amp;
using std::cout;

DelayNode::DelayNode(int id) : MiddleNode(id) {
    // lets implement the audio info class first
}

DelayNode::~DelayNode() { }

void DelayNode::showGui() {

    ImGui::PushItemWidth(100);
    imnodes::PushColorStyle(imnodes::ColorStyle_TitleBar, IM_COL32(80,80,80, 100));
    imnodes::PushColorStyle(imnodes::ColorStyle_TitleBarSelected, IM_COL32(80,80,80, 255));
    imnodes::PushColorStyle(imnodes::ColorStyle_TitleBarHovered, IM_COL32(80,80,80, 255));
    
    imnodes::BeginNode(id);
        
        imnodes::BeginNodeTitleBar();
            ImGui::TextUnformatted("Delay");
        imnodes::EndNodeTitleBar();

        ImGui::TextColored(ImVec4(ImColor(150,70,70,255)), "This node WILL cause segfaults and potentially damaging audio. For your safety, this node has been disabled.");

        ImGui::DragFloat("Delay (seconds)", &(this->time_delay), 0.1, 0, 10, "%.3f s");

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

void DelayNode::ApplyFX(const float *in, float *out, size_t numFrames, const AudioInfo &info) {

}