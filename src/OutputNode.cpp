#include <OutputNode.hpp>

using namespace guitar_amp;

OutputNode::~OutputNode() {
    
}

float OutputNode::getGain() {
    return this->gain;
}

void OutputNode::setGain(float g) {
    this->gain = g;
}

void OutputNode::showGui() {

    ImGui::PushItemWidth(100);
    ImNodes::PushColorStyle(ImNodesCol_TitleBar, IM_COL32(20, 30, 200, 100));
    ImNodes::PushColorStyle(ImNodesCol_TitleBarSelected, IM_COL32(20, 30, 200, 255));
    ImNodes::PushColorStyle(ImNodesCol_TitleBarHovered, IM_COL32(20, 30, 200, 255));
    
    ImNodes::BeginNode(id);
        ImNodes::BeginNodeTitleBar();
            ImGui::TextUnformatted("Output");
        ImNodes::EndNodeTitleBar();
        ImNodes::PushAttributeFlag(ImNodesAttributeFlags_EnableLinkDetachWithDragClick);
        ImNodes::BeginInputAttribute(this->id+1);
        ImNodes::EndInputAttribute();
        ImNodes::PopAttributeFlag();
    ImNodes::EndNode();

    ImGui::PopItemWidth();
    ImNodes::PopColorStyle();
    ImNodes::PopColorStyle();

}