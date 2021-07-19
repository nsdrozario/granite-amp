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


#endif