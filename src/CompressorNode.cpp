#include <CompressorNode.hpp>
using namespace guitar_amp;

CompressorNode::CompressorNode(int id) : MiddleNode(id) { }

CompressorNode::~CompressorNode() { }

void CompressorNode::showGui() {
    ImGui::PushItemWidth(100);
    imnodes::PushColorStyle(imnodes::ColorStyle_TitleBar, IM_COL32(184,73,0,255));
    imnodes::PushColorStyle(imnodes::ColorStyle_TitleBarSelected, IM_COL32(255, 111, 0, 255));
    imnodes::PushColorStyle(imnodes::ColorStyle_TitleBarHovered, IM_COL32(255, 111, 0, 255));

    imnodes::BeginNode(this->id);

        imnodes::BeginNodeTitleBar();
            ImGui::TextUnformatted("Compressor");
        imnodes::EndNodeTitleBar();
        imnodes::PushAttributeFlag(imnodes::AttributeFlags::AttributeFlags_EnableLinkDetachWithDragClick);
        imnodes::BeginInputAttribute(this->id+1);
        imnodes::EndInputAttribute();

        ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(ImColor(209,192,8)));
        ImGui::Text("Note: This node's functionality is unimplemented. This will merely pass its input into its output.");
        ImGui::PopStyleColor();

        ImGui::DragFloat("Attack", &this->attack, 0.01f, 0.0f, 10.0f, "%.3f s");
        ImGui::DragFloat("Release", &this->release, 0.01f, 0.0f, 10.0f, "%.3f s");
        ImGui::DragFloat("Ratio", &this->ratio, 0.01, 1.0f, 50.0f, "%.3f");
        ImGui::DragFloat("Threshold", &this->threshold, 1.0f, -1440.0f, 0.0f, "%.3f dB");

        if (this->sidechain_enabled) {
            imnodes::BeginInputAttribute(this->id+2);
                ImGui::Text("Sidechain Signal");
            imnodes::EndInputAttribute();
        }

        ImGui::Checkbox("Sidechain", &this->sidechain_enabled);

        imnodes::BeginOutputAttribute(this->id+3);
        imnodes::EndOutputAttribute();
        imnodes::PopAttributeFlag();
    imnodes::EndNode();

    ImGui::PopItemWidth();
    imnodes::PopColorStyle();
    imnodes::PopColorStyle();
}

void CompressorNode::ApplyFX(const float *in, float *out, size_t numFrames) {

    memcpy(out, in, numFrames * sizeof(float));

}