#include <DelayNode.hpp>
#include <algorithm>
#include <iostream>
#include <internal_dsp.hpp>
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

        ImGui::DragFloat("Delay (seconds)", &(this->time_delay), 0.1, 0, 10, "%.3f s");


    imnodes::EndNode();

    ImGui::PopItemWidth();
    imnodes::PopColorStyle();
    imnodes::PopColorStyle();

}

void DelayNode::ApplyFX(const float *in, float *out, size_t numFrames, AudioInfo info) {
    
    float needed_samples_delay = dsp::seconds_to_samples(time_delay, info.sample_rate);
    
    if (samples_delay != needed_samples_delay) {
        samples_delay = needed_samples_delay;
    }

    // check if the buffer needs to be reinitialized
    bool need_reinit = false;
    if ((info != internal_info)){
        need_reinit = true;
        internal_info = info;
    }
    if (buf.size() != (samples_delay + internal_info.period_length)) {
        need_reinit = true;
    }

    if (need_reinit) {
        buf.reinit(samples_delay + internal_info.period_length, 0, samples_delay);
    }

    for (size_t i = 0; i < numFrames; i++) {
        // write to ring buffer
        buf.set_write_ptr_value(in[i] * 0.5); // make the duplicated signal quieter
        buf.inc_write_ptr();
    }

    for (size_t i = 0; i < numFrames; i++) {
        out[i] = in[i] + buf.get_read_ptr_value();
        buf.inc_read_ptr();
    }
    
}