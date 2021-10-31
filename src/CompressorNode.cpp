#include <CompressorNode.hpp>
#include <state.hpp>
#include <imknob.hpp>

using namespace guitar_amp;

CompressorNode::CompressorNode(int id, const AudioInfo current_audio_info) : MiddleNode(id, current_audio_info) { }
CompressorNode::CompressorNode(int id, const AudioInfo current_audio_info, const sol::table &init_table) : MiddleNode(id, current_audio_info) { 
    /*
        Example config:
        {
            ["Threshold"]=0,
            ["Ratio"]=1
        }
    */
    threshold = init_table.get_or("Threshold", 0.0);
    ratio = init_table.get_or("Ratio", 1.0);
}

void CompressorNode::luaInit(const sol::table &init_table) {
    threshold = init_table.get_or("Threshold", 0.0);
    ratio = init_table.get_or("Ratio", 1.0);  
}

sol::table CompressorNode::serializeLua() {
    sol::table out;
    sol::table state;
    out["type"] = "Compressor";

    state["Threshold"] = threshold;
    state["Ratio"] = ratio;

    out["state"] = state;
    return out;
}

CompressorNode::~CompressorNode() { }

void CompressorNode::showGui() {
    ImNodes::PushColorStyle(ImNodesCol_TitleBar, IM_COL32(184,73,0,255));
    ImNodes::PushColorStyle(ImNodesCol_TitleBarSelected, IM_COL32(255, 111, 0, 255));
    ImNodes::PushColorStyle(ImNodesCol_TitleBarHovered, IM_COL32(255, 111, 0, 255));
    ImGui::PushItemWidth(100);
    ImNodes::BeginNode(this->id);

        ImNodes::BeginNodeTitleBar();
            ImGui::TextUnformatted("Compressor");
        ImNodes::EndNodeTitleBar();
        ImNodes::PushAttributeFlag(ImNodesAttributeFlags_EnableLinkDetachWithDragClick);
        ImNodes::BeginInputAttribute(this->id+1, ImNodesPinShape_TriangleFilled);
        ImNodes::EndInputAttribute();
        ImNodes::BeginOutputAttribute(this->id+3, ImNodesPinShape_TriangleFilled);
        ImNodes::EndOutputAttribute();
        ImNodes::PopAttributeFlag();
        /*
        ImGui::DragFloat("Attack", &this->attack, 0.01f, 0.0f, 10.0f, "%.3f s");
        ImGui::DragFloat("Release", &this->release, 0.01f, 0.0f, 10.0f, "%.3f s");
        */

        if (advancedMode) {

            ImGui::DragFloat("Ratio", &this->ratio, 0.01, 1.0f, 50.0f, "%.3f");
            ImGui::DragFloat("Threshold", &this->threshold, 1.0f, -60.0f, 0.0f, "%.3f dB");

        } else {

            ImKnob::Knob("Ratio", &this->ratio, 0.01, 1.0f, 50.0f, "%.1f", 24.0f, ImVec4(0.1f,0.1f,0.1f,1.0f), ImVec4(0.15f,0.15f,0.15f,1.0f));
            ImGui::SameLine();
            ImKnob::Knob("Threshold", &this->threshold, 1.0f, -60.0f, 0.0f, "%.0f dB", 24.0f, ImVec4(0.1f,0.1f,0.1f,1.0f), ImVec4(0.15f,0.15f,0.15f,1.0f));

        }

        /*
        // consider replacing with LFO
        if (this->sidechain_enabled) {
        
        ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(ImColor(255, 255, 102)));
        ImGui::Text("Note: Sidechaining is unimplemented for the time being.\nNothing different will happen if you attach a node here!");
        ImGui::PopStyleColor();

            ImNodes::BeginInputAttribute(this->id+2, ImNodesPinShape_TriangleFilled);
                ImGui::Text("Sidechain Signal");
            ImNodes::EndInputAttribute();
        }

        ImGui::Checkbox("Sidechain", &this->sidechain_enabled);
        */
    ImNodes::EndNode();
    ImNodes::PopColorStyle();
    ImNodes::PopColorStyle();
    ImGui::PopItemWidth();
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