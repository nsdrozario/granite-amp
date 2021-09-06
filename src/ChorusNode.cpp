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
        max_samples_delay = dsp::seconds_to_samples(max_delay / 1000.0f, static_cast<float>(info.sample_rate));
        min_samples_delay = dsp::seconds_to_samples(min_delay / 1000.0f, static_cast<float>(info.sample_rate));
        delay_line.resize(max_samples_delay);
    }

    rand_dist = std::uniform_real_distribution<double>(0.0, 1.0);
    rng = std::default_random_engine(rand_device());

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
        ImNodes::BeginInputAttribute(this->id+1);
        ImNodes::EndInputAttribute();

        ImNodes::BeginOutputAttribute(this->id+3);
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
    }

    for (size_t i = 0; i < numFrames; i++) {

        float delay = rand_dist(rng);
        float real_delay_samples = static_cast<float>(max_samples_delay - min_samples_delay) * delay;
        real_delay_samples += static_cast<float>(min_samples_delay);
        out[i] = in[i] + delay_line.read_tap_lerp(real_delay_samples);
        delay_line.push(in[i]);

    }

}