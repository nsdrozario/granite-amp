#include <DelayNode.hpp>
#include <algorithm>
#include <iostream>
#include <internal_dsp.hpp>
#include <cmath>
#include <imknob.hpp>

using namespace guitar_amp;
using std::cout;

DelayNode::DelayNode(int id, const AudioInfo current_audio_info) : MiddleNode(id, current_audio_info) {
}

DelayNode::DelayNode(int id, const AudioInfo current_audio_info, const sol::table &init_table) : MiddleNode(id, current_audio_info) {
    /*
        example config:
        {
            ["DelayTimeSeconds"]=0,
            ["FeedbackGain"]=-6
        }
    */
   time_delay = init_table.get_or("DelayTimeSeconds", 0.0);
   feedback_gain = init_table.get_or("FeedbackGain", -6.0);
}

DelayNode::~DelayNode() { }

void DelayNode::luaInit(const sol::table &init_table) {
   time_delay = init_table.get_or("DelayTimeSeconds", 0.0);
   feedback_gain = init_table.get_or("FeedbackGain", -6.0);   
}

sol::table DelayNode::serializeLua() {
    sol::table out;
    sol::table state;
    out["type"] = "Delay";
    state["DelayTimeSeconds"] = time_delay;
    state["FeedbackGain"] = feedback_gain;
    out["state"] = state;
    return out;
}

void DelayNode::showGui() {
    ImGui::PushItemWidth(100);
    ImNodes::PushColorStyle(ImNodesCol_TitleBar, IM_COL32(80,80,80, 100));
    ImNodes::PushColorStyle(ImNodesCol_TitleBarSelected, IM_COL32(80,80,80, 255));
    ImNodes::PushColorStyle(ImNodesCol_TitleBarHovered, IM_COL32(80,80,80, 255));
    
    ImNodes::BeginNode(id);
        
        ImNodes::BeginNodeTitleBar();
            ImGui::TextUnformatted("Delay");
        ImNodes::EndNodeTitleBar();

        ImNodes::PushAttributeFlag(ImNodesAttributeFlags_EnableLinkDetachWithDragClick);
        ImNodes::BeginInputAttribute(this->id+1, ImNodesPinShape_TriangleFilled);
        ImNodes::EndInputAttribute();

        ImNodes::BeginOutputAttribute(this->id+3, ImNodesPinShape_TriangleFilled);
        ImNodes::EndOutputAttribute();
        ImNodes::PopAttributeFlag();

       // ImGui::TextColored(ImVec4(ImColor(150,70,70,255)), "This node WILL cause segfaults and potentially damaging audio. For your safety, this node has been disabled.");

        if (advancedMode) {
            ImGui::DragFloat("Delay (seconds)", &(this->time_delay), 0.1, 0, 10, "%.3f s");
            // for now this is disabled because it makes controlling the feedback weird
            // ImGui::DragFloat("Delay Gain", &delay_gain, 1.0f, -144.0f, 0.0f, "%.3f dB");
            ImGui::DragFloat("Feedback Gain", &feedback_gain, 1.0f, -144.0f, -1.0f, "%.3f dB");
        } else {
            ImKnob::Knob("Delay (seconds)", &time_delay, 0.1, 0, 10, "%.1f s", 24.0f, ImVec4(0.1f,0.1f,0.1f,1.0f), ImVec4(0.15f,0.15f,0.15f,1.0f));
            ImGui::SameLine();
            ImKnob::Knob("Feedback Gain",  &feedback_gain, 1.0f, -144.0f, -1.0f, "%.0f dB", 24.0f, ImVec4(0.1f,0.1f,0.1f,1.0f), ImVec4(0.15f,0.15f,0.15f,1.0f));
        }
    ImNodes::EndNode();
    ImGui::PopItemWidth();
    ImNodes::PopColorStyle();
    ImNodes::PopColorStyle();

}

void DelayNode::ApplyFX(const float *in, float *out, size_t numFrames, AudioInfo info) {
    float needed_samples_delay = 0;
    
    if (time_delay > 10.0f) {
        time_delay = 10.0f;
    }

    if (std::isfinite(time_delay)) {
        needed_samples_delay = dsp::seconds_to_samples(time_delay, info.sample_rate);
    } else {
        time_delay = 0.0f;
    }

    if (samples_delay != needed_samples_delay) {
        samples_delay = needed_samples_delay;
    }

    // check if the buffer needs to be reinitialized
    if (dsp::seconds_to_samples(time_delay, info.sample_rate) != 0) {
        bool need_reinit = false;
        if ((info != internal_info)){
            need_reinit = true;
            internal_info = info;
        }

        if (buf.size() != samples_delay) {
            need_reinit = true;
        }

        if (need_reinit) {
            if (buf.size() < samples_delay) {
                buf.reinit(samples_delay, 0, 0);
            } else {
                size_t write_ptr = buf.get_write_ptr_index();
                size_t read_ptr = write_ptr - samples_delay + buf.size();
                read_ptr %= buf.size();
                buf.set_delay_no_resize(samples_delay);
            }
        }

        for (size_t i = 0; i < numFrames; i++) {
            float rb_val = buf.get_read_ptr_value();
            buf.inc_read_ptr();
            out[i] = in[i] + rb_val;
            buf.set_write_ptr_value( dsp::dbfs_to_f32(feedback_gain) * (rb_val + in[i]) );
            buf.inc_write_ptr();
        }
    } else {
        memcpy(out, in, numFrames * sizeof(float));
    }
}