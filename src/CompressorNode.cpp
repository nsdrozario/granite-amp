#include <CompressorNode.hpp>
#include <state.hpp>
#include <imknob.hpp>

using namespace guitar_amp;

CompressorNode::CompressorNode(int id, const AudioInfo current_audio_info) : MiddleNode(id, current_audio_info) { }

CompressorNode::~CompressorNode() { }

void CompressorNode::showGui() {
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
        imnodes::BeginOutputAttribute(this->id+3);
        imnodes::EndOutputAttribute();
        imnodes::PopAttributeFlag();
        /*
        ImGui::DragFloat("Attack", &this->attack, 0.01f, 0.0f, 10.0f, "%.3f s");
        ImGui::DragFloat("Release", &this->release, 0.01f, 0.0f, 10.0f, "%.3f s");
        */

        if (advancedMode) {

            ImGui::DragFloat("Ratio", &this->ratio, 0.01, 1.0f, 50.0f, "%.3f");
            ImGui::DragFloat("Threshold", &this->threshold, 1.0f, -1440.0f, 0.0f, "%.3f dB");

        } else {

            ImKnob::Knob("Ratio", &this->ratio, 0.01, 1.0f, 50.0f, "%.1f", 24.0f, ImVec4(0.1f,0.1f,0.1f,1.0f), ImVec4(0.15f,0.15f,0.15f,1.0f));
            ImGui::SameLine();
            ImKnob::Knob("Threshold", &this->threshold, 1.0f, -1440.0f, 0.0f, "%.0f dB", 24.0f, ImVec4(0.1f,0.1f,0.1f,1.0f), ImVec4(0.15f,0.15f,0.15f,1.0f));

        }

        /*
        // consider replacing with LFO
        if (this->sidechain_enabled) {
        
        ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(ImColor(255, 255, 102)));
        ImGui::Text("Note: Sidechaining is unimplemented for the time being.\nNothing different will happen if you attach a node here!");
        ImGui::PopStyleColor();

            imnodes::BeginInputAttribute(this->id+2);
                ImGui::Text("Sidechain Signal");
            imnodes::EndInputAttribute();
        }

        ImGui::Checkbox("Sidechain", &this->sidechain_enabled);
        */
    imnodes::EndNode();
    imnodes::PopColorStyle();
    imnodes::PopColorStyle();
}

void CompressorNode::ApplyFX(const float *in, float *out, size_t numFrames, AudioInfo info) {
    float linear_threshold = dsp::dbfs_to_f32(this->threshold);
    memcpy(out, in, numFrames*sizeof(float));
    for (size_t i = 0; i < numFrames; i++) {
        if (out[i] > linear_threshold) {
            out[i] = ((out[i]-linear_threshold) / dsp::dbfs_to_f32(ratio)) + linear_threshold;
        } else if (-(out[i]) > linear_threshold) {
            out[i] = ((out[i] + linear_threshold) / dsp::dbfs_to_f32(ratio)) - linear_threshold;
        }
    }
}