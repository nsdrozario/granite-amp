
#include <ChorusNode.hpp>
#include <algorithm>
#include <iostream>
#include <internal_dsp.hpp>
#include <cmath>
#include <imknob.hpp>
#include <state.hpp>

using namespace guitar_amp;
using std::cout;

ChorusNode::ChorusNode(int id, const AudioInfo info) : MiddleNode(id, info) {

    if (globalAudioInfo.sample_rate == 0) {
        
        for (int i = 0; i < CHORUS_VOICES; i++) {
            std::size_t buf_size = dsp::seconds_to_samples(80.0f/1000.0f, 48000);
            delay_lines[i].resize(buf_size);
            delay_lines[i].reset_data();
        }

    } else {

        for (int i = 0; i < CHORUS_VOICES; i++) {
            std::size_t buf_size = dsp::seconds_to_samples(80.0f/1000.0f, globalAudioInfo.sample_rate);
            delay_lines[i].resize(buf_size);
            delay_lines[i].reset_data();
        }

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

        for (int i = 0; i < CHORUS_VOICES; i++) {
            if (advancedMode) {
                ImGui::Text("Voice %d", i);
                if (ImGui::DragFloat("Avg. Delay", &(delay_line_settings[i].avg_delay), 1.0f, 1.0f, 50.0f, "%.1f ms")) {
                    need_reinit = true;
                }
                if (ImGui::DragFloat("Width", &(delay_line_settings[i].width), 1.0f, 0.0f, delay_line_settings[i].avg_delay-1, "%.1f ms")) {
                    need_reinit = true;
                }
                if (ImGui::DragFloat("Frequency", &(delay_line_settings[i].frequency), 1.0f, 0.1f, 2.0f, "%.1f Hz")) {
                    need_reinit = true;
                }
                if (ImGui::DragFloat("Depth", &(delay_line_settings[i].depth), 1.0f, 0.0f, 100.0f, "%.0f%%")) {
                    need_reinit = true;
                }
                ImGui::NewLine();
            } else {
                ImGui::Text("Voice %d", i);
                if (ImKnob::Knob("Avg. Delay", &(delay_line_settings[i].avg_delay), 1.0f, 1.0f, 50.0f, "%.1f ms", 18.0f, COLOR_KNOB_DARK, COLOR_KNOB_DARK_SELECTED)) {
                    need_reinit = true;
                }
                ImGui::SameLine();
                if(ImKnob::Knob("Width", &(delay_line_settings[i].width), 1.0f, 0.0f, delay_line_settings[i].avg_delay-1, "%.1f ms", 18.0f, COLOR_KNOB_DARK, COLOR_KNOB_DARK_SELECTED)) {
                    need_reinit = true;
                }
                ImGui::SameLine();
                if (ImKnob::Knob("Frequency", &(delay_line_settings[i].frequency), 1.0f, 0.1f, 2.0f, "%.1f Hz", 18.0f, COLOR_KNOB_DARK, COLOR_KNOB_DARK_SELECTED)) {
                    need_reinit = true;
                }
                ImGui::SameLine();
                if (ImKnob::Knob("Depth", &(delay_line_settings[i].depth), 1.0f, 0.0f, 100.0f, "%.0f%%", 18.0f, COLOR_KNOB_DARK, COLOR_KNOB_DARK_SELECTED)) {
                    need_reinit = true;
                }
                ImGui::NewLine();
            }
        }

        ImNodes::BeginOutputAttribute(this->id+3, ImNodesPinShape_TriangleFilled);
        ImNodes::EndOutputAttribute();
        ImNodes::PopAttributeFlag();



    ImNodes::EndNode();
    ImGui::PopItemWidth();
    ImNodes::PopColorStyle();
    ImNodes::PopColorStyle();

}

void ChorusNode::ApplyFX(const float *in, float *out, size_t numFrames, AudioInfo info) {
    
    if (internal_info != globalAudioInfo) {
            for (int i = 0; i < CHORUS_VOICES; i++) {
                std::size_t buf_size = dsp::seconds_to_samples(100.0f/1000.0f, globalAudioInfo.sample_rate);
                delay_lines[i].resize(buf_size);
            }
        } else {
            for (int i = 0; i < CHORUS_VOICES; i++) {
                std::size_t buf_size = dsp::seconds_to_samples(100.0f/1000.0f, 48000);
                delay_lines[i].resize(buf_size);
            }
        internal_info = globalAudioInfo;
    }
    
    if (need_reinit) {
        internal_timer = 0;
        for (int i = 0; i < CHORUS_VOICES; i++) {
            delay_lines[i].reset_data();
        }
        need_reinit = false;
    }


    for (size_t i = 0; i < numFrames; i++) {
        float lfo = (dsp::seconds_to_samples(delay_line_settings[0].width/1000.0f, globalAudioInfo.sample_rate) * std::sin(2.0f * 3.141592654f * delay_line_settings[0].frequency * internal_timer / globalAudioInfo.sample_rate));
        lfo += dsp::seconds_to_samples(delay_line_settings[0].avg_delay, globalAudioInfo.sample_rate);
        out[i] = ((1-delay_line_settings[0].depth) * in[i]) + ((delay_line_settings[0].depth) * delay_lines[0].read_tap_lerp(lfo));
    }

}

void ChorusNode::luaInit(const sol::table &settings) {
    /* 
        {
            ["voices"]={
                {
                    ["avg_delay"]=20,
                    ["width"]=5,
                    ["depth"]=50,
                    ["freq"]=0.7
                },
                {
                    ["avg_delay"]=20,
                    ["width"]=5,
                    ["depth"]=50,
                    ["freq"]=0.7
                }
            }
        }
    */

    int voices_counted = 0;
    for (auto &o : settings) {
        
        if (voices_counted > CHORUS_VOICES) {
            break;
        }
        
        sol::object &key = o.first;
        sol::object &val = o.second;

        sol::table voice = val.as<sol::table>();

        delay_line_settings[voices_counted].avg_delay = voice.get_or("avg_delay", 20.0f);
        delay_line_settings[voices_counted].depth = voice.get_or("depth",50.0f);
        delay_line_settings[voices_counted].width = voice.get_or("width", 5.0f);
        delay_line_settings[voices_counted].frequency = voice.get_or("freq", 0.7);

        voices_counted++;
    }

}

sol::table ChorusNode::serializeLua() {
    sol::table out = sol::table();
    out["type"] = "Chorus";
    out["state"] = sol::table();
    return out;
}
