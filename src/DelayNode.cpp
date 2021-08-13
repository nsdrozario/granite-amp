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

DelayNode::~DelayNode() { }

void DelayNode::showGui() {
    ImGui::PushItemWidth(100);
    imnodes::PushColorStyle(imnodes::ColorStyle_TitleBar, IM_COL32(80,80,80, 100));
    imnodes::PushColorStyle(imnodes::ColorStyle_TitleBarSelected, IM_COL32(80,80,80, 255));
    imnodes::PushColorStyle(imnodes::ColorStyle_TitleBarHovered, IM_COL32(80,80,80, 255));
    
    imnodes::BeginNode(id);
        
        imnodes::BeginNodeTitleBar();
            ImGui::TextUnformatted("Delay");
        imnodes::EndNodeTitleBar();

        imnodes::PushAttributeFlag(imnodes::AttributeFlags::AttributeFlags_EnableLinkDetachWithDragClick);
        imnodes::BeginInputAttribute(this->id+1);
        imnodes::EndInputAttribute();

        imnodes::BeginOutputAttribute(this->id+3);
        imnodes::EndOutputAttribute();
        imnodes::PopAttributeFlag();

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
    imnodes::EndNode();
    ImGui::PopItemWidth();
    imnodes::PopColorStyle();
    imnodes::PopColorStyle();

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
                buf.set_delay_no_resize(delay_s)
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