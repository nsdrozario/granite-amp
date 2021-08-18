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
    ImNodes::PushColorStyle(ImNodesCol_TitleBar, IM_COL32(40, 150, 40, 100));
    ImNodes::PushColorStyle(ImNodesCol_TitleBarSelected, IM_COL32(40,150,40,255));
    ImNodes::PushColorStyle(ImNodesCol_TitleBarHovered, IM_COL32(40,150,40,255));
    
    ImNodes::BeginNode(id);
        ImNodes::BeginNodeTitleBar();
            ImGui::TextUnformatted("Input");
        ImNodes::EndNodeTitleBar();
        ImNodes::PushAttributeFlag(ImNodesAttributeFlags_EnableLinkDetachWithDragClick);
        ImNodes::BeginOutputAttribute(this->id+3);
        ImNodes::EndOutputAttribute();
        ImNodes::PopAttributeFlag();
    ImNodes::EndNode();

    ImGui::PopItemWidth();
    ImNodes::PopColorStyle();
    ImNodes::PopColorStyle();

}