/*
Use this regex to change everything to SliderFloats from DragFloats:
ImGui::DragFloat\( *([_"a-zA-Z0-9 \.>+\-&%\(\)\/]+) *, *([_ "a-zA-Z0-9\.>+\-&%\(\)\/]+), *([_\/ "a-zA-Z0-9\.>+\-&%\(\)]+), *([_\/ "a-zA-Z0-9\.>+\-&%\(\)]+), *([_\/ "a-zA-Z0-9\.>+\-&%\(\)]+), *([_\/ "a-zA-Z0-9\.>+\-&%\(\)]+);
ImGui::SliderFloat($1, $2, $4, $5, $6)
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

#include <Nodes.hpp>

#include <utility>
#include <set>
#include <vector>
#include <unordered_set>
#include <stack>
#include <state.hpp>
#include <chrono>
#include <sstream>

#include <internal_dsp.hpp>
#include <iostream>
#include <io_util.hpp>
#include <implot.h>

#include <imknob.hpp>
#include <immeter.hpp>

#include <AudioInfo.hpp>
#include <MainUI.hpp>

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
bool audioEnabled = true;

std::string dfs_path = "";

std::vector<const char *> inputNames;
std::vector<const char *> outputNames;

guitar_amp::AudioInfo globalAudioInfo;
std::mutex globalAudioInfoMutex;

// this is really not worth managing memory over
// this might need to be a ring buffer
std::vector<float> metronomeTickSound; 
dsp::ring_buffer<float> metronomeRingBuffer;

bool metronomeEnabled;
int metronomeBPM = 120;
bool oversamplingEnabled = true;

float *tmp_output = nullptr;
float *tmp_input = nullptr;
float *output_buf = nullptr;

size_t metronomeSamplesPassed = 0;
size_t metronomeTickSamples = 0;
bool metronomeOn = false;
float metronomeGainDB = 0.0f;

ma_encoder audioRecorder;
bool recordingAudio = false;
imgui_addons::ImGuiFileBrowser recorderFileBrowser;
std::mutex recorderMutex;

float processTime = 0.0f;
bool advancedMode = false;
int current_node = 10;

std::vector<std::string> config_paths;
std::vector<std::string> config_names;

bool *config_selected = nullptr;
int config_selected_id = 1;

sf::Texture amp_grill;
sf::Sprite amp_grill_sprite;

std::mutex nodes_mutex;

float rms_value = 0;
int rms_count = 0;

bool warning_open = true;
float global_gain = 100.0f;

void callback(ma_device *d, void *output, const void *input, ma_uint32 numFrames) {

    ma_uint32 buffer_size_in_bytes = numFrames * ma_get_bytes_per_frame(d->capture.format, d->capture.channels);
    
    AudioInfo newInfo;
    newInfo.channels = d->capture.channels;
    newInfo.period_length = numFrames;
    newInfo.sample_rate = d->sampleRate;

    if (audioEnabled) {
        // check if audio settings have changed compared to last block of frames
        bool needReallocation = false;
        if (newInfo != globalAudioInfo) {
            // update the global audio settings
            globalAudioInfoMutex.lock();
            globalAudioInfo = newInfo;
            globalAudioInfoMutex.unlock();
            
            needReallocation = true;

            // read file assets/metronome_tick.wav
            metronomeTickSound = io::read_entire_file_wav("assets/metronome_tick.wav", globalAudioInfo.sample_rate);
            metronomeRingBuffer.reinit(metronomeTickSound.size(), 0, 0);
            for (size_t i = 0; i < metronomeTickSound.size(); i++) {
                metronomeRingBuffer.set_write_ptr_value(metronomeTickSound[i]);
                metronomeRingBuffer.inc_write_ptr();
            }

        }

        if (needReallocation || !tmp_input || !output_buf || !tmp_output) {
            delete[] tmp_input;
            delete[] tmp_output;
            delete[] output_buf;
            tmp_input = new float[numFrames];
            tmp_output = new float[numFrames];
            output_buf = new float[numFrames];
        }
       
        const float *f32_input = static_cast<const float *> (input);
        float *f32_output = static_cast<float *> (output);
        
        memcpy(tmp_input, f32_input, buffer_size_in_bytes);
        memcpy(output_buf, tmp_input, buffer_size_in_bytes);

        // depth first search from vertex 3 (the output attribute of the input node)
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
            int current_node_dfs = (current_attribute / 5) * 5;
            int next_attribute = -1;
            int next_node = -1;
            
            if (!visited_nodes[current_node_dfs]) {
                visited_nodes[current_node_dfs] = true;
                current_dfs_path << current_node_dfs << " -> ";
                if (current_node_dfs == 5) {
                    reached_end = true;
                    goto processing_done;
                }
                // if the current vertex is a valid node
                MiddleNode *currentNodePtr = dynamic_cast<MiddleNode *>(nodes[current_node_dfs]);
                if (currentNodePtr) {
                    // apply that node's effects
                    currentNodePtr->ApplyFX(tmp_input, tmp_output, numFrames, globalAudioInfo); 
                    // copy the output of the node's effects and recycle it
                    memcpy(output_buf, tmp_output, buffer_size_in_bytes);
                    memcpy(tmp_input, tmp_output, buffer_size_in_bytes);
                }
                dfs_stack.push(adjlist[current_node_dfs+3]);
            }
        }
        
        processing_done:
        // this is really just for debugging
        if (current_dfs_path.str() != dfs_path) {
            dfs_path = current_dfs_path.str();
            std::cout << current_dfs_path.str() << std::endl;
        }
        
        if (reached_end) {
            // sync output_buf with output to return the processed data
            MA_COPY_MEMORY(output, output_buf, buffer_size_in_bytes);
        }

        if (recordingAudio) {
            recorderMutex.lock();
            ma_encoder_write_pcm_frames(&audioRecorder, output, numFrames);
            recorderMutex.unlock();
        }

        if (metronomeEnabled) {
            if (metronomeTickSamples == 0) {
                /*  
                    60 bpm = 1 beat per second
                    120 bpm = 2 beats per second
                    180 bpm = 3 beats per second
                    ...
                    n bpm = n/60 beats per second

                    1 beat per second = 1 second of delay between beats
                    2 beats per second = 0.5 seconds of delay between beats
                    4 beats per second = 0.25 seconds of delay between beats
                    ...
                    n beats per second = 1/n seconds of delay between beats

                    therefore 60/bpm = seconds of delay between each beat for a metronome
                */
                metronomeTickSamples = dsp::seconds_to_samples(60.0f / static_cast<float>(metronomeBPM), globalAudioInfo.sample_rate);
                metronomeSamplesPassed = 0;
            }
            
            size_t metronomeIterator = 0;

            // how many samples do we have until we need to output a metronome click?
            // if the click won't occur in this period
            if (metronomeTickSamples - metronomeSamplesPassed >= globalAudioInfo.period_length) {
                // we can simply skip this period of frames
                metronomeSamplesPassed += globalAudioInfo.period_length;
            // otherwise we will have to worry about it
            } else if (metronomeTickSamples - metronomeSamplesPassed < globalAudioInfo.period_length) {
                metronomeOn = true;
                // the sound will start at this difference of frame counts
                metronomeIterator = metronomeTickSamples - metronomeSamplesPassed;
                metronomeSamplesPassed = 0;
            }

            if (metronomeOn) {
                
                size_t frames_read = 0;
                while 
                (
                    metronomeIterator < globalAudioInfo.period_length 
                    && metronomeIterator < metronomeTickSamples 
                    && (metronomeRingBuffer.get_read_ptr_index() != 0 || frames_read == 0)
                ) 
                {
                    frames_read++;
                    f32_output[metronomeIterator++] += (metronomeRingBuffer.get_read_ptr_value() * dsp::dbfs_to_f32(metronomeGainDB));
                    metronomeRingBuffer.inc_read_ptr();
                    metronomeSamplesPassed++;
                }

                if (metronomeRingBuffer.get_read_ptr_index() == 0) {
                    metronomeOn = false;
                }

            }

        }

        for (std::size_t i = 0; i < numFrames; i++) {
            f32_output[i] *= global_gain/100.0f;
        }

        if (rms_count == 0) {
            rms_value = 0;
        
            for (std::size_t i = 0; i < numFrames; i++) {
                rms_value += f32_output[i] * f32_output[i];
            }

            rms_value /= numFrames;
            rms_value = dsp::f32_to_dbfs(std::sqrt(rms_value));
        }
        rms_count++;
        rms_count %= 1;
    }

}

bool default_settings_exist = false;

void print_adjlist() {
    std::cout <<"adjlist: \n";
    for (auto it = adjlist.begin(); it != adjlist.end(); it++) {
        std::cout << it->first << "->" << it->second << std::endl;
    }
}

std::vector<const char *> config_paths_c_str;
std::vector<const char *> config_names_c_str;


int main () {

    std::ifstream check_default ("default_imgui.ini");
    if (check_default) {
        default_settings_exist = true;
    }

    // Initialize Miniaudio
    {
    if (ma_context_init(NULL, 0, NULL, &context) != MA_SUCCESS) {
        std::cout << "miniaudio init failed\n";
        exit(-1);
    }

    io::refresh_devices();
    io::file_paths(config_paths, "assets/signalchain_presets/");
    io::file_names(config_names, "assets/signalchain_presets");
    config_selected = new bool[config_names.size()];
    io::align_c_str_vector(config_paths, config_paths_c_str);
    io::align_c_str_vector(config_names, config_names_c_str);

    deviceConfig = ma_device_config_init(ma_device_type_duplex);
    deviceConfig.periodSizeInFrames = 512;
    deviceConfig.capture.channels = 1;
    deviceConfig.capture.format = ma_format_f32;
    deviceConfig.playback.channels = 1;
    deviceConfig.playback.format = ma_format_f32;
    deviceConfig.dataCallback = callback; 
    }
    // Initialize ImGui
    sf::Event e;
    sf::RenderWindow w(sf::VideoMode::getDesktopMode(), "GraniteAmp");
    sf::Clock dt;
    
    // get images
    sf::Texture bg;
    
    if (!bg.loadFromFile("assets/board.png")) {
        std::cout << "error loading board.png\n";
    }

    if(!amp_grill.loadFromFile("assets/amp_grill.png")) {
        std::cout << "error loading amp_grill.png\n";
    }

    amp_grill.setRepeated(true);
    amp_grill_sprite.setTexture(amp_grill);

    bg.setRepeated(true);
    sf::Sprite bgSprite;
    bgSprite.setTexture(bg);

    ImGui::SFML::Init(w);
    ImGui::Spectrum::StyleColorsSpectrum();
    ImNodes::CreateContext();
    ImPlot::CreateContext();
    ImGuiStyle &style = ImGui::GetStyle();
    ImGuiIO& io = ImGui::GetIO();
    io.Fonts->Clear();
    ImFont *font = io.Fonts->AddFontFromFileTTF("fonts/SourceSansPro-Black.ttf", 20.0f);
    ImGui::SFML::UpdateFontTexture();
    ImNodes::GetStyle().GridSpacing = 120;
    ImNodes::GetStyle().NodeCornerRounding = 10.0f;
    ImNodes::GetStyle().PinCircleRadius = 7.0f;
    ImNodes::GetStyle().PinTriangleSideLength = 14.0f;
    style.WindowRounding = 10.0f;
    style.ChildRounding = 10.0f;
    style.PopupRounding = 10.0f;
    // if vsync isn't enabled the app will use way too much GPU time
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
    

    ImNodes::SetNodeEditorSpacePos(0, ImVec2(50,100));
    ImNodes::SetNodeEditorSpacePos(5, ImVec2(150, 100));
    ImNodes::SetNodeEditorSpacePos(-1, ImVec2(720,670));
    ImNodesIO &imnodes_io = ImNodes::GetIO();
    imnodes_io.EmulateThreeButtonMouse.Modifier = &io.KeyAlt;
    imnodes_io.LinkDetachWithModifierClick.Modifier = &io.KeyCtrl;

    amp_load_preset("assets/signalchain_presets/Clean.lua");

    while (w.isOpen()) {
        while (w.pollEvent(e)) {
            ImGui::SFML::ProcessEvent(e);
            if (e.type == sf::Event::Closed) {
                for (auto it = nodes.begin(); it != nodes.end(); it++) {
                    delete it->second;
                }
                w.close();
            }
        }

        w.clear();        
        ImGui::SFML::Update(w, dt.restart());
        // resize the background if necessary
        if (w.getSize().x > bgSprite.getTextureRect().width || w.getSize().y > bgSprite.getTextureRect().height) {
            bgSprite.setTextureRect(sf::IntRect(0, 0, w.getSize().x + bgSprite.getTextureRect().width, w.getSize().y + bgSprite.getTextureRect().height));
        }

        // draw the background
        w.draw(bgSprite);

        if (warning_open) {
            ImGui::OpenPopup("Warning Popup");    
        }
        if (ImGui::BeginPopupModal("Warning Popup", &warning_open)) {
            ImGui::TextColored(ImColor(IM_COL32(179, 50, 41, 255)), "WARNING: This app contains rapidly changing colors,\nwhich may cause seizures in some people.\nUsers should exercise caution.\nThis is not medical advice; please consult a medical\nprofessional for medical advice.");
            ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(179.f/255, 50.f/255, 41.f/255, 1));
            ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(235.f/255, 64.f/255, 52.f/255, 1));
            ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(235.f/255, 64.f/255, 52.f/255, 1));
            if (ImGui::Button("I acknowledge this warning.")) {
                warning_open = false;
            }
            ImGui::PopStyleColor();
            ImGui::PopStyleColor();
            ImGui::PopStyleColor();
            ImGui::EndPopup();
        }
        // imgui stuff
        // draw node 
        ImGui::PushFont(font);
        if (!default_settings_exist) {
            ImGui::SetNextWindowSize(ImVec2(sf::VideoMode::getDesktopMode().width * 0.7, sf::VideoMode::getDesktopMode().height * 0.9));
            ImGui::SetNextWindowPos(ImVec2(0,0));
        }
        ImGui::Begin("Signal Chain");
        ImNodes::BeginNodeEditor();
            // draw nodes
            static bool help_open = true;
            ImNodes::PushColorStyle(ImNodesCol_TitleBar, IM_COL32(117, 161, 150, 255));
            ImNodes::PushColorStyle(ImNodesCol_TitleBarHovered, IM_COL32(117, 161, 150, 255));
            ImNodes::PushColorStyle(ImNodesCol_TitleBarSelected, IM_COL32(117, 161, 150, 255));
            ImGui::PushItemWidth(100);
            ImNodes::BeginNode(-1);
                ImNodes::BeginNodeTitleBar();
                    ImGui::Text("Help");
                ImNodes::EndNodeTitleBar();
                if (help_open) {
                    ImGui::Text("- Drag the triangle pins to each other to connect nodes.");
                    ImGui::Text("- Nodes can be disconnected by dragging a connection out of a triangle pin.");
                    ImGui::Text("- Nodes can also be disconnected by holding Ctrl while clicking on a connection.");
                    ImGui::Text("- Right click the grid to add a node.");
                    ImGui::Text("- Right click a node and click \"Delete Node\" to delete it.");
                    ImGui::Text("- Scroll around the grid by holding Alt while dragging the mouse with left click,\nor drag with the mouse wheel button held down.");
                }
                ImGui::Checkbox("Show", &help_open);
            ImNodes::EndNode();
            ImNodes::PopColorStyle();
            ImGui::PopItemWidth();

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
                    ImNodes::Link((it->first/5)*5, it->first, it->second);
                }
            }
        ImNodes::MiniMap(0.2f, 3);
        ImNodes::EndNodeEditor();

        // node editing popups
        ui::node_popups();

        ImGui::End();
        
        int start_link; int end_link;
        if (ImNodes::IsLinkCreated(&start_link, &end_link)) {
            if (adjlist.find(start_link) != adjlist.end() || adjlist.find(end_link) != adjlist.end()) {
                std::cout << "Deleting link from " << start_link << "->" << adjlist[start_link]  << std::endl;
                adjlist.erase(start_link);
                adjlist_inward.erase(end_link);
            }
            std::cout << "Create " << start_link << " -> " << end_link << std::endl;
            adjlist[start_link] = end_link;
            adjlist_inward[end_link] = start_link;
            /*
                std::cout <<"adjlist: \n";
                for (auto it = adjlist.begin(); it != adjlist.end(); it++) {
                    std::cout << it->first << "->" << it->second << std::endl;
                }
            */
           print_adjlist();
        }
        int destroyed_link = -1;
        if (ImNodes::IsLinkDestroyed(&destroyed_link)) {
            int forward = adjlist[destroyed_link+3];
            adjlist.erase(destroyed_link+3);
            adjlist_inward.erase(forward);
            print_adjlist();
        }
        if (!default_settings_exist) {
            ImGui::SetNextWindowPos(ImVec2(sf::VideoMode::getDesktopMode().width*0.7 + 10, 0));
            ImGui::SetNextWindowSize(ImVec2(sf::VideoMode::getDesktopMode().width * 0.3 - 15, sf::VideoMode::getDesktopMode().height * 0.3 - 10));
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
        if (!default_settings_exist) {
            ImGui::SetNextWindowPos(ImVec2(sf::VideoMode::getDesktopMode().width*0.7 + 10, sf::VideoMode::getDesktopMode().height * 0.3 ));
            ImGui::SetNextWindowSize(ImVec2(sf::VideoMode::getDesktopMode().width * 0.3 - 10, sf::VideoMode::getDesktopMode().height * 0.3 - 10));
        }
        ImGui::Begin("Metronome");
            ImGui::Checkbox("Enabled", &metronomeEnabled);
            
            if (ImGui::InputInt("BPM", &metronomeBPM, 1, 10)) {
                metronomeTickSamples = 0;
            }
            if (advancedMode) {
                ImGui::DragFloat("Gain", &metronomeGainDB, 1.0f, -60.0f, 6.0f, "%.3f dB");
            } else {
                ImKnob::Knob("Gain", &metronomeGainDB, 1.0f, -60.0f, 12.0f, "%.0f dB", 24.0f, COLOR_KNOB_DARK, COLOR_KNOB_DARK_SELECTED);
            } 
        ImGui::End();
        if (!default_settings_exist) {
            ImGui::SetNextWindowPos(ImVec2(sf::VideoMode::getDesktopMode().width*0.85 + 15, sf::VideoMode::getDesktopMode().height * 0.6 ));
            ImGui::SetNextWindowSize(ImVec2(sf::VideoMode::getDesktopMode().width * 0.15 - 20, sf::VideoMode::getDesktopMode().height * 0.3 - 5));
        }
        ImGui::Begin("Preset Manager");
            // preset loader
            if (ImGui::ListBox("Presets", &config_selected_id, config_names_c_str.data(), config_names.size())) {
                // load preset
                amp_load_preset(config_paths[config_selected_id]);
            }

            /*
            if(ImGui::Button("Save Preset")) {
                std::cout << "save" << std::endl;
            }
            */

            if (ImGui::Button("Refresh Preset List")) {
                io::file_paths(config_paths, "assets/signalchain_presets/");
                io::file_names(config_names, "assets/signalchain_presets/");
                if (config_selected != nullptr) {
                    delete[] config_selected;
                }
                config_selected = new bool[config_names.size()];
                io::align_c_str_vector(config_paths, config_paths_c_str);
                io::align_c_str_vector(config_names, config_names_c_str);
            }
        ImGui::End();
        if (!default_settings_exist) {
            ImGui::SetNextWindowPos(ImVec2(sf::VideoMode::getDesktopMode().width*0.7 + 10, sf::VideoMode::getDesktopMode().height * 0.6 ));
            ImGui::SetNextWindowSize(ImVec2(sf::VideoMode::getDesktopMode().width * 0.15, sf::VideoMode::getDesktopMode().height * 0.3 - 5));
        }
        // control panel
        ImGui::Begin("Control Panel");

            // ImGui::Text("Time to process: %.1f ms", processTime);
            static float fake_volume = -60;
            if (!warning_open) {
                ImMeter::Meter("Volume (dBFS)", &rms_value, -60.0, 0.0);
            } else {
                ImMeter::Meter("Volume (dBFS)", &fake_volume, -60.0, 0.0);
            }

            ImGui::SliderFloat("Volume", &global_gain, 0.0f, 100.0f, "%.0f%%");

            if (audioEnabled) {
            
                if (recordingAudio) {
                    ImGui::PushStyleColor(ImGuiCol_Button, IM_COL32(209,30,17,255));
                    ImGui::PushStyleColor(ImGuiCol_ButtonHovered, IM_COL32(255,54,54,255));
                    ImGui::PushStyleColor(ImGuiCol_ButtonActive, IM_COL32(255,54,54,255));
                } else {    
                    ImGui::PushStyleColor(ImGuiCol_Button, IM_COL32(145,33,25,255));
                    ImGui::PushStyleColor(ImGuiCol_ButtonHovered, IM_COL32(255,54,54,255));
                    ImGui::PushStyleColor(ImGuiCol_ButtonActive, IM_COL32(255,54,54,255));
                }

                if (!recordingAudio && ImGui::Button("Record")) {
                    ImGui::OpenPopup("Recorder File Explorer");
                } else if (recordingAudio && ImGui::Button("Stop Recording")) {
                    recorderMutex.lock();
                    ma_encoder_uninit(&audioRecorder);
                    recorderMutex.unlock();
                    recordingAudio = false;
                }

                ImGui::PopStyleColor(3);
                
                if (
                    recorderFileBrowser.showFileDialog(
                        "Recorder File Explorer", 
                        imgui_addons::ImGuiFileBrowser::DialogMode::SAVE, 
                        ImVec2(300,200), 
                        ".wav"
                    )
                ) 
                {
                    ma_encoder_config recorderConfig = ma_encoder_config_init(ma_resource_format_wav, ma_format_f32, 1, globalAudioInfo.sample_rate);
                    recorderMutex.lock();
                    if (ma_encoder_init_file(recorderFileBrowser.selected_fn.c_str(), &recorderConfig, &audioRecorder) == MA_SUCCESS) {
                        recordingAudio = true;
                    }
                    recorderMutex.unlock();
                }

                if (recordingAudio) {
                    ImGui::TextColored(sf::Color::Red, "Recording audio to %s", recorderFileBrowser.selected_fn.c_str());
                }

            }
            // todo: levels meter
            if (advancedMode) {
            //    ImGui::Checkbox("Oversampled Overdrive (4x)", &oversamplingEnabled);
            }

            ImGui::Checkbox("Advanced Mode", &advancedMode);
            
            if (ImGui::Button("Restore Default Layout")) {
                ImGui::LoadIniSettingsFromDisk("default_imgui.ini");
                ImGui::SaveIniSettingsToDisk("imgui.ini");
            }

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

            std::cout << "new device connected" << std::endl;
            std::cout << "sample rate: " << device.sampleRate << std::endl;
            std::cout << "capture sample rate: " << device.capture.internalSampleRate << std::endl;
            std::cout << "playback sample rate: " << device.playback.internalSampleRate << std::endl;
        }
 
        if (!default_settings_exist) {
            ImGui::SaveIniSettingsToDisk("default_imgui.ini");
            default_settings_exist = true;
        }

        ImGui::PopFont();
        ImGui::SFML::Render(w);

        w.display();

    }

    ImPlot::DestroyContext();
    ImNodes::DestroyContext();
    ImGui::SFML::Shutdown();

    return 0;
}