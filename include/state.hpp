#ifndef GUITAR_AMP_STATE_HPP
#define GUITAR_AMP_STATE_HPP

#include "headers.hpp"
#include <vector>
#include <string>
#include "AudioProcessorNode.hpp"
#include <map>
#include <unordered_map>
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

extern std::vector<std::string> inputNames;
extern std::vector<std::string> outputNames;

extern const char ** cstr_inputNames;
extern const char ** cstr_outputNames;

extern std::map<int, AudioProcessorNode *> nodes;
extern std::map<int,int> adjlist; // will need another adjacency list to track inward links to prevent double connections on an attribute
extern std::map<int,int> adjlist_inward;

extern int current_edge_id;
extern ma_uint32 lastFrameCount;
extern bool audioEnabled;

#endif