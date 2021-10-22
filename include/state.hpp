#ifndef GUITAR_AMP_STATE_HPP
#define GUITAR_AMP_STATE_HPP

#include "headers.hpp"
#include <vector>
#include <string>
#include "AudioProcessorNode.hpp"
#include <map>
#include <mutex>
#include <unordered_map>
#include <AudioInfo.hpp>
#include <sol/sol.hpp>

using namespace guitar_amp;

// global vars
// define these in amp.cpp only
extern ma_context context;
extern ma_device_info *inputDevices;
extern ma_device_info *outputDevices;
extern ma_uint32 numInputDevices;
extern ma_uint32 numOutputDevices;
extern ma_device device;
extern ma_device_config deviceConfig;

extern int listBoxSelectedInput;
extern int listBoxSelectedOutput;

extern std::map<int, AudioProcessorNode *> nodes;
extern std::map<int,int> adjlist; // will need another adjacency list to track inward links to prevent double connections on an attribute
extern std::map<int,int> adjlist_inward;

extern int current_edge_id;
extern ma_uint32 lastFrameCount;
extern bool audioEnabled;

extern std::vector<const char *> inputNames;
extern std::vector<const char *> outputNames;

extern std::mutex globalAudioInfoMutex;
extern guitar_amp::AudioInfo globalAudioInfo;
extern bool metronomeEnabled;
extern bool oversamplingEnabled;

extern bool advancedMode;
extern int current_node;
extern int last_node_hovered;

extern std::vector<std::string> config_paths;
extern std::vector<std::string> config_names;
extern bool *config_selected;
extern int config_selected_id;

std::string adjlist_to_lua();
std::string nodes_to_lua();

void lua_to_adjlist(const sol::table &adjlist_p, const sol::table &adjlist_inward_p);
void lua_to_nodes(const sol::table &data);
void amp_load_preset(const std::string &name);
void amp_save_preset(const std::string &name);

extern std::mutex nodes_mutex;

extern sf::Texture amp_grill;
extern sf::Sprite amp_grill_sprite;

#endif