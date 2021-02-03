#include <InputNode.hpp>

using namespace guitar_amp;

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

        ImGui::DragFloat("Gain coefficient", &(this->gain), 0.1, 0, 20, "%.3f");

        imnodes::BeginOutputAttribute(this->id);
        imnodes::EndOutputAttribute();
    imnodes::EndNode();

    ImGui::PopItemWidth();
    imnodes::PopColorStyle();
    imnodes::PopColorStyle();

}