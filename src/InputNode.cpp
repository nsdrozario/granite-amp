#include <InputNode.hpp>

using namespace guitar_amp;

InputNode::~InputNode() {
    
}

float InputNode::getGain() {
    return this->gain;
}

void InputNode::setGain(float g) {
    this->gain = g;
}

void InputNode::showGui() {

    ImGui::PushItemWidth(100);
    imnodes::PushColorStyle(imnodes::ColorStyle_TitleBar, IM_COL32(40, 150, 40, 100));
    imnodes::PushColorStyle(imnodes::ColorStyle_TitleBarSelected, IM_COL32(40,150,40,255));
    imnodes::PushColorStyle(imnodes::ColorStyle_TitleBarHovered, IM_COL32(40,150,40,255));
    
    imnodes::BeginNode(id);
        imnodes::BeginNodeTitleBar();
            ImGui::TextUnformatted("Input");
        imnodes::EndNodeTitleBar();
        imnodes::PushAttributeFlag(imnodes::AttributeFlags::AttributeFlags_EnableLinkDetachWithDragClick);
        imnodes::BeginOutputAttribute(this->id+3);
        imnodes::EndOutputAttribute();
        imnodes::PopAttributeFlag();
    imnodes::EndNode();

    ImGui::PopItemWidth();
    imnodes::PopColorStyle();
    imnodes::PopColorStyle();

}