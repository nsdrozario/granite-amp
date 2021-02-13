/*
    Real time guitar amplifier simulation
    Copyright (C) 2021  Nathaniel D'Rozario

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License along
    with this program; if not, write to the Free Software Foundation, Inc.,
    51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
*/
#define MINIAUDIO_IMPLEMENTATION
#include "headers.hpp"
#include <kfr/all.hpp>

#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <cmath>
#include <algorithm>
#include <SFML/Graphics.hpp>

#include "imgui.h"
#include "imgui-SFML.h"
#include "imnodes.h"

#include <AudioProcessorNode.hpp>
#include <InputNode.hpp>
#include <OutputNode.hpp>
#include <OverdriveNode.hpp>
#include <ConvolutionNode.hpp>

#include <utility>
#include <set>
#include <vector>
#include <unordered_set>

#define AMP_SAMPLE_RATE 48000

#include <internal_dsp.hpp>
#include <iostream>
using namespace guitar_amp;

// global vars
ma_context context;
ma_device_info *inputDevices;
ma_device_info *outputDevices;
ma_uint32 numInputDevices;
ma_uint32 numOutputDevices;

int listBoxSelectedInput = 0;
int listBoxSelectedOutput = 0;

std::vector<std::string> inputNames;
std::vector<std::string> outputNames;

const char ** cstr_inputNames;
const char ** cstr_outputNames;

int main () {

    // Initialize Miniaudio
    
    if (ma_context_init(NULL, 0, NULL, &context) != MA_SUCCESS) {
        std::cout << "miniaudio init failed\n";
        exit(-1);
    }

    ma_result result_get_devices = ma_context_get_devices
    (
        &context,
        &outputDevices,
        &numOutputDevices,
        &inputDevices,
        &numInputDevices
    );

    if (result_get_devices != MA_SUCCESS) {
        std::cout << "Unable to obtain audio device info\n";
        exit(-1);
    }

    cstr_inputNames = new const char *[numInputDevices];
    cstr_outputNames = new const char *[numOutputDevices];

    for (ma_uint32 i = 0; i < numInputDevices; i++) {
        cstr_inputNames[i] = inputDevices[i].name;
        std::cout << inputDevices[i].name << "\n";
    }

    for (ma_uint32 i = 0; i < numOutputDevices; i++) {
        cstr_outputNames[i] = outputDevices[i].name;
        std::cout << outputDevices[i].name << "\n";
    }


    // Initialize ImGui
    sf::Event e;
    sf::RenderWindow w(sf::VideoMode(800,600), "Guitar Amp");
    sf::Clock dt;
    ImGui::SFML::Init(w);
    imnodes::Initialize();

    /*
        Each node will reserve 5 times it's own zero-indexed ID, plus 4 more spaces.
        The first two IDs after the node's ID * 5 are reserved for input, and the next two for output.
        For example:
        Node 0:
            ID: 0
            Input: 1, 2
            Output: 3, 4
        Node 1:
            ID: 5
            Input: 6, 7
            Output: 8, 9
    */
    std::map<int, AudioProcessorNode *> nodes;
    std::vector<std::unordered_set<int>> adjlist_outward;
    std::vector<std::unordered_set<int>> adjlist_inward;
    std::vector<std::pair<int,int>> edge_list;

    nodes[0] = new guitar_amp::InputNode(0);
    nodes[5] = new guitar_amp::OutputNode(5);

    int current_node = 10;

    while (w.isOpen()) {
        while (w.pollEvent(e)) {
            ImGui::SFML::ProcessEvent(e);
            if (e.type == sf::Event::Closed) {
                for (auto it = nodes.begin(); it != nodes.end(); it++) {
                    delete it->second;
                }
                imnodes::Shutdown();
                ImGui::SFML::Shutdown();
                w.close();
            }
        }
        w.clear();
        ImGui::SFML::Update(w, dt.restart());

        // imgui stuff

        // draw node editor
        imnodes::BeginNodeEditor();

            // node creation

            bool should_open_popup = ImGui::IsWindowFocused(ImGuiFocusedFlags_RootAndChildWindows) && imnodes::IsEditorHovered() && ImGui::IsMouseReleased(ImGuiMouseButton_Right);
            if (!ImGui::IsAnyItemHovered() && should_open_popup) {
                ImGui::OpenPopup("Node Creator");
            }

            if (ImGui::BeginPopup("Node Creator")) {

                if (ImGui::MenuItem("Create Overdrive Node")) {
                    nodes[current_node] = new guitar_amp::OverdriveNode(current_node);
                    current_node += 5;
                }

                if (ImGui::MenuItem("Create Convolution IR Node")) {
                    nodes[current_node] = new guitar_amp::ConvolutionNode(current_node);
                    current_node += 5;
                }

                ImGui::EndPopup();
            }
        
            // draw nodes

            for (auto it = nodes.begin(); it != nodes.end(); it++) {
                int node_id = it->first;
                AudioProcessorNode *node = it->second;
                if (node != nullptr) {
                    node->showGui();
                }
            }

            // draw links
            for (size_t i = 0; i < adjlist_outward.size(); i++) {
                int starting_node = (i/static_cast<size_t>(5))*static_cast<size_t>(5);
                for (auto t : adjlist_outward[i]) {
                    imnodes::Link((starting_node/5)* 5, i, t);
                }
            }

        imnodes::EndNodeEditor();

        int start_link; int end_link;
        if (imnodes::IsLinkCreated(&start_link, &end_link)) {
            int new_size = std::max(start_link, end_link);
            if (new_size > adjlist_inward.size()) {
                adjlist_inward.resize(new_size+1);
            }
            if (new_size > adjlist_outward.size()) {
                adjlist_outward.resize(new_size+1);
            }
            adjlist_outward[start_link].insert(end_link);
            adjlist_inward[end_link].insert(start_link);
            edge_list.push_back(std::pair<int,int>(start_link,end_link));
        }

        int destroyedLink;
        if (imnodes::IsLinkDestroyed(&destroyedLink)) {
            adjlist_outward[edge_list[destroyedLink-1].first].erase(edge_list[destroyedLink-1].second);
            adjlist_inward[edge_list[destroyedLink-1].second].erase(edge_list[destroyedLink-1].first);
        }

        ImGui::Begin("I/O Devices");
            ImGui::ListBox("Inputs", &listBoxSelectedInput, cstr_inputNames, static_cast<int>(numInputDevices));
            ImGui::ListBox("Outputs", &listBoxSelectedOutput, cstr_outputNames, static_cast<int>(numOutputDevices));
        ImGui::End();


        // raw sfml calls

        ImGui::SFML::Render(w);
        w.display();
    }

    return 0;
}

