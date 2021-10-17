
#include <ChorusNode.hpp>
#include <algorithm>
#include <iostream>
#include <internal_dsp.hpp>
#include <cmath>
#include <imknob.hpp>
#include <random>

using namespace guitar_amp;
using std::cout;

ChorusNode::ChorusNode(int id, const AudioInfo info) : MiddleNode(id, info) {

    if (info.sample_rate == 0) {

    } else {
        max_samples_delay = dsp::seconds_to_samples(max_delay / 1000.0f, info.sample_rate);
        min_samples_delay = dsp::seconds_to_samples(min_delay / 1000.0f, info.sample_rate);
        lfo_amplitude_samples = dsp::seconds_to_samples(lfo_amplitude/1000.0f, info.sample_rate);
        delay_line.resize(max_samples_delay + lfo_amplitude_samples + 1);
    }

}

ChorusNode::~ChorusNode() { }

void ChorusNode::showGui() {
    ImGui::PushItemWidth(100);
    ImNodes::PushColorStyle(ImNodesCol_TitleBar, IM_COL32(3, 157, 252, 100));
    ImNodes::PushColorStyle(ImNodesCol_TitleBarSelected, IM_COL32(3, 157, 252, 255));
    ImNodes::PushColorStyle(ImNodesCol_TitleBarHovered, IM_COL32(3, 157, 252, 255));
    
    ImNodes::BeginNode(id);
        
        ImNodes::BeginNodeTitleBar();
            ImGui::TextUnformatted("Chorus");
        ImNodes::EndNodeTitleBar();

        ImNodes::PushAttributeFlag(ImNodesAttributeFlags_EnableLinkDetachWithDragClick);
        ImNodes::BeginInputAttribute(this->id+1, ImNodesPinShape_TriangleFilled);
        ImNodes::EndInputAttribute();

        ImNodes::BeginOutputAttribute(this->id+3, ImNodesPinShape_TriangleFilled);
        ImNodes::EndOutputAttribute();
        ImNodes::PopAttributeFlag();



    ImNodes::EndNode();
    ImGui::PopItemWidth();
    ImNodes::PopColorStyle();
    ImNodes::PopColorStyle();

}

void ChorusNode::ApplyFX(const float *in, float *out, size_t numFrames, AudioInfo info) {
    
    if (internal_info != info) {
        size_t samples_delay = dsp::seconds_to_samples(max_delay / 1000.0f, static_cast<float>(info.sample_rate));
        delay_line.resize(samples_delay);
        internal_info = info;
        max_samples_delay = dsp::seconds_to_samples(max_delay / 1000.0f, info.sample_rate);
        min_samples_delay = dsp::seconds_to_samples(min_delay / 1000.0f, info.sample_rate);
        lfo_amplitude_samples = dsp::seconds_to_samples(lfo_amplitude/1000.0f, info.sample_rate);
    }


    for (size_t i = 0; i < numFrames; i++) {

        float lfo_tap1 = std::sin(2 * 3.141592654f * tap1_freq * internal_timer / static_cast<float>(info.sample_rate)) * lfo_amplitude_samples;
        float lfo_tap2 = std::sin(2 * 3.141592654f * tap2_freq * internal_timer / static_cast<float>(info.sample_rate)) * lfo_amplitude_samples;
        out[i] = in[i] + delay_line.read_tap_lerp(max_samples_delay + lfo_tap1) + delay_line.read_tap_lerp(min_samples_delay + lfo_tap2);
        delay_line.push(in[i]);

        internal_timer = std::fmod(internal_timer + 1, info.sample_rate);

    }

}

void ChorusNode::luaInit(const sol::table &settings) {

}

sol::table ChorusNode::serializeLua() {
    sol::table out = sol::table();
    out["type"] = "Chorus";
    out["state"] = sol::table();
    return out;
}
