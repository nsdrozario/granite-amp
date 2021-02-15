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
#include <headers.hpp>
#include <climits>

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
#include <stack>
#include <state.hpp>

#include <internal_dsp.hpp>
#include <iostream>
using namespace guitar_amp;

// global vars
ma_context context;
ma_device_info *inputDevices;
ma_device_info *outputDevices;
ma_uint32 numInputDevices;
ma_uint32 numOutputDevices;
ma_device device;
ma_device_config deviceConfig;

int listBoxSelectedInput = 0;
int listBoxSelectedOutput = 0;

std::vector<std::string> inputNames;
std::vector<std::string> outputNames;

const char ** cstr_inputNames;
const char ** cstr_outputNames;

std::map<int, AudioProcessorNode *> nodes;
std::map<int,int> adjlist; // will need another adjacency list to track inward links to prevent double connections on an attribute

int current_edge_id = INT_MIN;
ma_uint32 lastFrameCount;
bool audioEnabled = false;

void callback(ma_device *d, void *output, const void *input, ma_uint32 numFrames) {
    if (numFrames != lastFrameCount) {
        std::cout << "period of " << numFrames << " frames" << std::endl;
        lastFrameCount = numFrames;
    }
    MA_ASSERT(d->capture.format == d->playback.format);
    MA_ASSERT(d->capture.channels == d->playback.channels);
    
    /*
    const float *f32_input = static_cast<const float *> (input);
    float *f32_output = static_cast<float *> (output);

    kfr::univector<float> u_input = kfr::make_univector(f32_input, ma_get_bytes_per_frame(d->capture.format, d->capture.channels));
    kfr::univector<float> u_output = kfr::univector<float>(u_input);
    kfr::univector<float> tmp_input = kfr::univector<float>(u_input);
    
    // dfs from vertex 3 (the output attribute of the input node)
    
    std::stack<int> dfs_stack;
    dfs_stack.push(3);
    std::unordered_map<int,bool> visited;
    while (!dfs_stack.empty()) {
        int current_node = dfs_stack.top();
        dfs_stack.pop();
        if (visited.find(current_node) != visited.end()) {
            if (visited[current_node]) {
                // do applyFX() at each step
                // if reached edge 6 stop (input attribute of the output node)
                // copy the output univector's data
            }
        }
    }
    */

    if (audioEnabled) {
        MA_COPY_MEMORY(output, input, numFrames * ma_get_bytes_per_frame(d->capture.format, d->capture.channels));
    }

}

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
    }

    for (ma_uint32 i = 0; i < numOutputDevices; i++) {
        cstr_outputNames[i] = outputDevices[i].name;
    }

    deviceConfig = ma_device_config_init(ma_device_type_duplex);
    deviceConfig.periodSizeInFrames = 512;
    deviceConfig.capture.channels = 1;
    deviceConfig.capture.format = ma_format_f32;
    deviceConfig.playback.channels = 1;
    deviceConfig.playback.format = ma_format_f32;
    deviceConfig.dataCallback = callback; 

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
            for (auto it = adjlist.begin(); it != adjlist.end(); it++) {
                imnodes::Link((it->first/5)*5, it->first, it->second);
            }

        imnodes::EndNodeEditor();

        int start_link; int end_link;
        if (imnodes::IsLinkCreated(&start_link, &end_link)) {
            if (adjlist.find(start_link) != adjlist.end() || adjlist.find(end_link) != adjlist.end()) {
                std::cout << "Deleting link from " << start_link << "->" << adjlist[start_link]  << std::endl;
                adjlist.erase(start_link);
            }
            std::cout << "Create " << start_link << " -> " << end_link << std::endl;
            adjlist[start_link] = end_link;
            std::cout <<"adjlist: \n";
            for (auto it = adjlist.begin(); it != adjlist.end(); it++) {
                std::cout << it->first << "->" << it->second << std::endl;
            }
        }

        /*
        int destroyedLink;
        if (imnodes::IsLinkDestroyed(&destroyedLink)) {
            std::cout << "Link destroyed\n";
            adjlist[edge_list[destroyedLink].first] = -1;
            adjlist[edge_list[destroyedLink].second] = -1;
            edge_list.erase(destroyedLink);
        }
        */

        // consider using ListBoxHeader
        ImGui::Begin("I/O Devices");
            ImGui::ListBox("Inputs", &listBoxSelectedInput, cstr_inputNames, static_cast<int>(numInputDevices));
            ImGui::ListBox("Outputs", &listBoxSelectedOutput, cstr_outputNames, static_cast<int>(numOutputDevices));
            ImGui::Checkbox("Audio enabled", &audioEnabled);
        ImGui::End();

        bool inputChanged = deviceConfig.capture.pDeviceID != &(inputDevices[listBoxSelectedInput].id);
        bool outputChanged = deviceConfig.playback.pDeviceID != &(outputDevices[listBoxSelectedOutput].id);
        
        if (inputChanged || outputChanged) {
            ma_device_stop(&device);
            ma_device_uninit(&device);
            if (inputChanged) {
                deviceConfig.capture.pDeviceID = &(inputDevices[listBoxSelectedInput].id);
            }
            if (outputChanged) {
                deviceConfig.playback.pDeviceID = &(outputDevices[listBoxSelectedOutput].id);
            }
            deviceConfig.sampleRate = 0; // default sample rate
            ma_device_init(&context, &deviceConfig, &device);
            ma_device_start(&device);
            std::cout << "sample rate: " << device.sampleRate << std::endl;
        }


        // raw sfml calls

        ImGui::SFML::Render(w);
        w.display();
    }

    return 0;
}

