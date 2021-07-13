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
#include <CompressorNode.hpp>
#include <AnalyzerNode.hpp>
#include <OscillatorNode.hpp>
#include <DelayNode.hpp>
#include <ShelfNode.hpp>
#include <CabSimNode.hpp>

#include <utility>
#include <set>
#include <vector>
#include <unordered_set>
#include <stack>
#include <state.hpp>
#include <sstream>

#include <internal_dsp.hpp>
#include <iostream>
#include <io_util.hpp>
#include <implot.h>

#include <AudioInfo.hpp>

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

std::map<int, AudioProcessorNode *> nodes;
std::map<int,int> adjlist; // will need another adjacency list to track inward links to prevent double connections on an attribute
std::map<int,int> adjlist_inward;

int current_edge_id = INT_MIN;
ma_uint32 lastFrameCount;
bool audioEnabled = false;

std::string dfs_path = "";

std::vector<const char *> inputNames;
std::vector<const char *> outputNames;

guitar_amp::AudioInfo globalAudioInfo;
std::mutex globalAudioInfoMutex;

void callback(ma_device *d, void *output, const void *input, ma_uint32 numFrames) {

    ma_uint32 buffer_size_in_bytes = numFrames * ma_get_bytes_per_frame(d->capture.format, d->capture.channels);
    
    globalAudioInfoMutex.lock();
    globalAudioInfo.channels = d->capture.channels;
    globalAudioInfo.period_length = numFrames;
    globalAudioInfo.sample_rate = d->sampleRate;
    globalAudioInfoMutex.unlock();

    if (audioEnabled) {
	    /*
        MA_ASSERT(d->capture.format == d->playback.format);
        MA_ASSERT(d->capture.channels == d->playback.channels);
        MA_ASSERT(d->capture.internalSampleRate == d->playback.internalSampleRate);
        MA_ASSERT(d->capture.internalPeriodSizeInFrames == d->playback.internalPeriodSizeInFrames);
        */
       
        const float *f32_input = static_cast<const float *> (input);
        float *f32_output = static_cast<float *> (output);

        float *tmp_input = new float[numFrames];
        float *tmp_output = new float[numFrames];
        float *output_buf = new float[numFrames];
        memcpy(tmp_input, f32_input, buffer_size_in_bytes);
        memcpy(output_buf, tmp_input, buffer_size_in_bytes);

        // dfs from vertex 3 (the output attribute of the input node)
        // this assumes that there will be only one inward connection per node, except for input and output (which is correct for the time being)
        // the audio processing has to be done on a node by node basis but the dfs operates on attribute to attribute
        std::stack<int> dfs_stack;
        dfs_stack.push(3);
        std::unordered_map<int,bool> visited_attributes;
        std::unordered_map<int,bool> visited_nodes;
        std::stringstream current_dfs_path;
        bool reached_end = false;
        while (!dfs_stack.empty()) {
            int current_attribute = dfs_stack.top(); dfs_stack.pop();
            int current_node = (current_attribute / 5) * 5;
            int next_attribute = -1;
            int next_node = -1;
            
            if (!visited_nodes[current_node]) {
                visited_nodes[current_node] = true;
                current_dfs_path << current_node << " -> ";
                if (current_node == 5) {
                    reached_end = true;
                    goto processing_done;
                }
                MiddleNode *currentNodePtr = dynamic_cast<MiddleNode *>(nodes[current_node]);
                if (currentNodePtr) {
                    currentNodePtr->ApplyFX(tmp_input, tmp_output, numFrames, globalAudioInfo); 
                    memcpy(output_buf, tmp_output, buffer_size_in_bytes);
                    memcpy(tmp_input, tmp_output, buffer_size_in_bytes);
                }
                dfs_stack.push(adjlist[current_node+3]);
            }
        }
        
        processing_done:
        if (current_dfs_path.str() != dfs_path) {
            dfs_path = current_dfs_path.str();
            std::cout << current_dfs_path.str() << std::endl;
        }
        if (reached_end) {
            MA_COPY_MEMORY(output, output_buf, buffer_size_in_bytes);
        }
        delete[] output_buf;
        delete[] tmp_input;
        delete[] tmp_output;
    }

}

int main () {

    // Initialize Miniaudio
    
    if (ma_context_init(NULL, 0, NULL, &context) != MA_SUCCESS) {
        std::cout << "miniaudio init failed\n";
        exit(-1);
    }

    io::refresh_devices();

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
    ImPlot::CreateContext();
    w.setVerticalSyncEnabled(true);
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
    ImGui::SetNextWindowSize(ImVec2(300,200));
    imnodes::SetNodeEditorSpacePos(0, ImVec2(50,100));
    imnodes::SetNodeEditorSpacePos(5, ImVec2(50, 200));
    while (w.isOpen()) {
        while (w.pollEvent(e)) {
            ImGui::SFML::ProcessEvent(e);
            if (e.type == sf::Event::Closed) {
                for (auto it = nodes.begin(); it != nodes.end(); it++) {
                    delete it->second;
                }
                imnodes::Shutdown();
                ImPlot::DestroyContext();
                ImGui::SFML::Shutdown();
                w.close();
            }
        }
        w.clear();
        ImGui::SFML::Update(w, dt.restart());

        // imgui stuff

        // draw node 
        ImGui::Begin("Signal Chain");
        imnodes::BeginNodeEditor();

            // node creation

            bool should_open_popup = ImGui::IsWindowFocused(ImGuiFocusedFlags_RootAndChildWindows) && imnodes::IsEditorHovered() && ImGui::IsMouseReleased(ImGuiMouseButton_Right);
            if (!ImGui::IsAnyItemHovered() && should_open_popup) {
                ImGui::OpenPopup("Node Creator");
            }

            if (ImGui::BeginPopup("Node Creator")) {
                
                if (ImGui::MenuItem("Create Overdrive Node")) {
                    nodes[current_node] = new guitar_amp::OverdriveNode(current_node, globalAudioInfo);
                    current_node += 5;
                }

                if (ImGui::MenuItem("Create Convolution IR Node")) {
                    nodes[current_node] = new guitar_amp::ConvolutionNode(current_node, globalAudioInfo);
                    current_node += 5;
                }

                if (ImGui::MenuItem("Create Compresoor Node")) {
                    nodes[current_node] = new guitar_amp::CompressorNode(current_node, globalAudioInfo);
                    current_node += 5;
                }

                if (ImGui::MenuItem("Create Analyzer Node")) {
                    nodes[current_node] = new guitar_amp::AnalyzerNode(current_node, globalAudioInfo);
                    current_node += 5;
                }

                if (ImGui::MenuItem("Create Shelf Node")) {
                    nodes[current_node] = new guitar_amp::ShelfNode(current_node, globalAudioInfo);
                    current_node += 5;
                }

                if (ImGui::MenuItem("Create Delay Node")) {
                    nodes[current_node] = new guitar_amp::DelayNode(current_node, globalAudioInfo);
                    current_node += 5;
                }

                #ifdef DEBUG_BUILD
                
                if (ImGui::MenuItem("Create Oscillator Node")) {
                    nodes[current_node] = new guitar_amp::OscillatorNode(current_node, globalAudioInfo);
                    current_node += 5;
                }

                if (ImGui::MenuItem("Create Cabinet Simulation Node")) {
                    nodes[current_node] = new guitar_amp::CabSimNode(current_node, globalAudioInfo);
                    current_node += 5;
                }

                #endif
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
                if (it->second != 0) {
                    imnodes::Link((it->first/5)*5, it->first, it->second);
                }
            }

        imnodes::EndNodeEditor();
        ImGui::End();

        int selected_node = 0;
        if (imnodes::IsNodeHovered(&selected_node) && ImGui::IsMouseReleased(ImGuiMouseButton_Right)) {
            if (selected_node != 0 && selected_node != 5) {
                ImGui::OpenPopup("Delete Node");
            }
        }

        if (ImGui::BeginPopup("Delete Node")) {

            if (ImGui::MenuItem("Delete Node")) {
                adjlist.erase(adjlist_inward[selected_node+1]);
                adjlist.erase(adjlist_inward[selected_node+2]);
                adjlist.erase(selected_node+3);
                adjlist.erase(selected_node+4);
                adjlist_inward.erase(selected_node+1);
                adjlist_inward.erase(selected_node+2);
                delete nodes[selected_node];
                nodes.erase(selected_node);
            }

            ImGui::EndPopup();
        }
        int start_link; int end_link;
        if (imnodes::IsLinkCreated(&start_link, &end_link)) {
            if (adjlist.find(start_link) != adjlist.end() || adjlist.find(end_link) != adjlist.end()) {
                std::cout << "Deleting link from " << start_link << "->" << adjlist[start_link]  << std::endl;
                adjlist.erase(start_link);
                adjlist_inward.erase(end_link);
            }
            std::cout << "Create " << start_link << " -> " << end_link << std::endl;
            adjlist[start_link] = end_link;
            adjlist_inward[end_link] = start_link;
            std::cout <<"adjlist: \n";
            for (auto it = adjlist.begin(); it != adjlist.end(); it++) {
                std::cout << it->first << "->" << it->second << std::endl;
            }
        }

        // consider using ListBoxHeader
        ImGui::Begin("I/O Devices");
            ImGui::ListBox("Inputs", &listBoxSelectedInput, inputNames.data(), static_cast<int>(numInputDevices));
            ImGui::ListBox("Outputs", &listBoxSelectedOutput, outputNames.data(), static_cast<int>(numOutputDevices));
            ImGui::Checkbox("Audio enabled", &audioEnabled);
            if (ImGui::Button("Refresh")) {
                io::refresh_devices(); // consider adding a mutex and putting this off to another thread
            }
        ImGui::End();

        // control panel
        ImGui::Begin("Control Panel");
            ImGui::Checkbox("Metronome", &metronomeEnabled);
            // todo; levels meter
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
            std::cout << "capture sample rate: " << device.capture.internalSampleRate << std::endl;
            std::cout << "playback sample rate: " << device.playback.internalSampleRate << std::endl;
        }


        // raw sfml calls

        ImGui::SFML::Render(w);
        w.display();
    }

    return 0;
}

