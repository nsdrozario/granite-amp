#include <OutputNode.hpp>

using namespace guitar_amp;

float OutputNode::getGain() {
    return this->gain;
}

void OutputNode::setGain(float g) {
    this->gain = g;
}

void OutputNode::showGui() {

    ImGui::PushItemWidth(100);
    imnodes::PushColorStyle(imnodes::ColorStyle_TitleBar, IM_COL32(20, 30, 200, 100));
    imnodes::PushColorStyle(imnodes::ColorStyle_TitleBarSelected, IM_COL32(20, 30, 200, 255));
    imnodes::PushColorStyle(imnodes::ColorStyle_TitleBarHovered, IM_COL32(20, 30, 200, 255));
    
    imnodes::BeginNode(id);
        imnodes::BeginNodeTitleBar();
            ImGui::TextUnformatted("Ouput");
        imnodes::EndNodeTitleBar();

        ImGui::DragFloat("Gain coefficient", &(this->gain), 0.1, 0, 20, "%.3f");

        imnodes::BeginInputAttribute(this->id);
        imnodes::EndInputAttribute();
    imnodes::EndNode();

    ImGui::PopItemWidth();
    imnodes::PopColorStyle();
    imnodes::PopColorStyle();

}