#include <DelayNode.hpp>
#include <algorithm>
#include <iostream>
using namespace guitar_amp;
using std::cout;
DelayNode::DelayNode(int id) : MiddleNode(id) {
    if (audioEnabled) {
        this->period_size = device.capture.internalPeriodSizeInFrames;
    } else {
        this->period_size = 512;      
    }
    this->samples_delay = 0;
    this->time_delay = 0.0f;
    this->write_ptr = 0;
    this->read_ptr = 0;

    this->ring_buffer = new float[this->samples_delay + this->period_size];
    this->write_ptr = this->samples_delay;
}

size_t DelayNode::inc_write_ptr() {
    this->write_ptr = (this->write_ptr + 1) % (this->samples_delay + this->period_size);
    return this->write_ptr;
}

size_t DelayNode::inc_read_ptr() {
    this->read_ptr = (this->read_ptr + 1) % (this->samples_delay + this->period_size);
    return this->read_ptr;
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

        ImGui::TextColored(ImVec4(ImColor(150,70,70,255)), "This node WILL cause segfaults and potentially damaging audio. For your safety, this node has been disabled.");

        ImGui::DragFloat("Delay (seconds)", &(this->time_delay), 0.1, 0, 10, "%.3f s");

        imnodes::PushAttributeFlag(imnodes::AttributeFlags::AttributeFlags_EnableLinkDetachWithDragClick);
        imnodes::BeginInputAttribute(this->id+1);
        imnodes::EndInputAttribute();

        imnodes::BeginOutputAttribute(this->id+3);
        imnodes::EndOutputAttribute();
        imnodes::PopAttributeFlag();
    imnodes::EndNode();

    ImGui::PopItemWidth();
    imnodes::PopColorStyle();
    imnodes::PopColorStyle();

}

void DelayNode::ApplyFX(const float *in, float *out, size_t num_frames) {

    // check if array needs to be resized

    size_t needed_samples_delay = dsp::seconds_to_samples(this->time_delay, device.sampleRate) != this->samples_delay && device.sampleRate);

    if 
    (
        (needed_samples_delay != this->samples_delay && device.sampleRate) || // updated sample time
        (num_frames != this->period_size)
    ) 
    {
        delete[] this->ring_buffer;
        this->ring_buffer = new float[num_frames + needed_samples_delay]();
        this->samples_delay = needed_samples_delay;
        this->write_ptr = this->samples_delay;
        this->read_ptr = 0;
    }

    


}