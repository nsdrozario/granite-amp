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
    this->ring_buffer.resize(this->period_size);
    std::fill(ring_buffer.begin(), ring_buffer.end(), 0.0f);
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

    // cout << "started one cycle\n";

    memcpy(out, in, num_frames * sizeof(float));
    size_t delay_in_samples = static_cast<size_t>(static_cast<float>(device.sampleRate) * this->time_delay);    
    
    if (this->period_size != device.capture.internalPeriodSizeInFrames) {
        this->period_size = device.capture.internalPeriodSizeInFrames;
    }
    
    if (this->samples_delay != delay_in_samples) {
        this->samples_delay = delay_in_samples;
    }
    
    if (this->ring_buffer.size() != this->period_size + this->samples_delay + 1) {
        this->ring_buffer.resize(this->period_size + this->samples_delay + 1);
        // reset ring buffer after resize to avoid inaccurate delay
        this->write_ptr = 0;
        this->read_ptr = 0; 
        std::fill(this->ring_buffer.begin(), this->ring_buffer.end(), 0.0f);
    }

    // fill in the ring buffer with the data offset by the delay in samples
    for 
    (
        size_t count = 0; 
        count < num_frames; 
        count++, 
        this->write_ptr = ((this->write_ptr + 1) % (this->samples_delay + this->period_size-1)) 
    ) 
    {
        this->ring_buffer[this->write_ptr] = in[count];
    }

    // add the current contents of the ring buffer to the output from the beginning of the ring buffer
    for 
    (
        size_t count = 0; 
        count < num_frames; 
        count++, 
        this->read_ptr = ((this->read_ptr + 1) % (this->samples_delay + this->period_size-1)) 
    ) 
    {
        // segfaults here occassionally
        out[count] = in[this->read_ptr];
    }

    // cout << "finished one cycle\n";

}