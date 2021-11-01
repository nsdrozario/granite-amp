#include <MainUI.hpp>
#include <imgui.h>
#include <imnodes.h>
#include <imknob.hpp>
#include <state.hpp>
#include <Nodes.hpp>
#include <iostream>
using namespace guitar_amp::ui;
int last_node_hovered = -1;

void delete_node(int selected_node) {
    if (selected_node != 0 && selected_node != 5 && selected_node > 0) {
        std::cout << "Deleting node " << selected_node << std::endl;
        adjlist.erase(adjlist_inward[selected_node+1]);
        adjlist.erase(adjlist_inward[selected_node+2]);
        adjlist.erase(selected_node+3);
        adjlist.erase(selected_node+4);
        adjlist_inward.erase(selected_node+1);
        adjlist_inward.erase(selected_node+2);
        delete nodes[selected_node];
        nodes.erase(selected_node); 
    }
}

void ui::node_popups() {
    int node_hovered = -1;
    bool is_node_hovered = ImNodes::IsNodeHovered(&node_hovered);
    bool node_creation_popup = ImGui::IsWindowHovered(ImGuiHoveredFlags_RootAndChildWindows);
    if (is_node_hovered) {
        last_node_hovered = node_hovered;
    }
    if (ImGui::IsMouseReleased(ImGuiMouseButton_Right)) {
        if (node_creation_popup && !is_node_hovered) {
            ImGui::OpenPopup("Node Creator");
        } else if (is_node_hovered) {
            if (node_hovered != 0 && node_hovered != 5 && node_hovered != -1) {
                ImGui::OpenPopup("Delete Node");
            }
        }
    }

    if (ImGui::BeginPopup("Node Creator")) {
        
        ImGui::TextColored(ImVec4(1.0f,1.0f,1.0f,0.5f), "Create Node:");

        if (ImGui::MenuItem("Ovedrive/Distortion")) {
            nodes[current_node] = new guitar_amp::OverdriveNode(current_node, globalAudioInfo);
            current_node += 5;
        }

        if (ImGui::MenuItem("Convolution Reverb")) {
            nodes[current_node] = new guitar_amp::ConvolutionNode(current_node, globalAudioInfo);
            current_node += 5;
        }

        if (ImGui::MenuItem("Compressor")) {
            nodes[current_node] = new guitar_amp::CompressorNode(current_node, globalAudioInfo);
            current_node += 5;
        }

        if (ImGui::MenuItem("Analyzer")) {
            nodes[current_node] = new guitar_amp::AnalyzerNode(current_node, globalAudioInfo);
            current_node += 5;
        }

        if (ImGui::MenuItem("Delay")) {
            nodes[current_node] = new guitar_amp::DelayNode(current_node, globalAudioInfo);
            current_node += 5;
        }

        if (ImGui::MenuItem("Cabinet Simulation")) {
            nodes[current_node] = new guitar_amp::CabSimNode(current_node, globalAudioInfo);
            current_node += 5;
        }

        if (ImGui::MenuItem("Simple EQ")) {
            nodes[current_node] = new guitar_amp::ThreeBandEQ(current_node, globalAudioInfo);
            current_node += 5;
        }

        if (ImGui::MenuItem("Flanger")) {
            nodes[current_node] = new guitar_amp::FlangerNode(current_node, globalAudioInfo);
            current_node += 5;
        }

        #ifdef DEBUG_BUILD
 
        /*
        if (ImGui::MenuItem("Chorus")) {
            nodes[current_node] = new guitar_amp::ChorusNode(current_node, globalAudioInfo);
            current_node += 5;
        }
        */

        if (ImGui::MenuItem("Oscillator")) {
            nodes[current_node] = new guitar_amp::OscillatorNode(current_node, globalAudioInfo);
            current_node += 5;
        }

        #endif
        ImGui::EndPopup();
    }

    if (ImGui::BeginPopup("Delete Node")) {
        if (ImGui::MenuItem("Delete Node")) {
            delete_node(last_node_hovered);
        }
        ImGui::EndPopup();
    }

}