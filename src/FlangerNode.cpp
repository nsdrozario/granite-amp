#include <FlangerNode.hpp>
using namespace guitar_amp;

FlangerNode::FlangerNode(int id, const AudioInfo current_audio_info) : MiddleNode(id, current_audio_info) {

    if (current_audio_info.sample_rate == 0) {
        // don't allocate
    } else {
        delay_buf_size = static_cast<size_t>(static_cast<float>(current_audio_info.sample_rate) * (MAX_DELAY_DURATION / 1000.0f) + 0.5);
        delay_buf = new float[delay_buf_size];
    }

}

FlangerNode::~FlangerNode() {
    if (delay_buf != nullptr) {
        delete[] delay_buf;
    }
}   

void FlangerNode::showGui() {

    ImGui::PushItemWidth(100);
    imnodes::PushColorStyle(imnodes::ColorStyle_TitleBar, IM_COL32(93,42,247, 100));
    imnodes::PushColorStyle(imnodes::ColorStyle_TitleBarSelected, IM_COL32(93,42,247, 255));
    imnodes::PushColorStyle(imnodes::ColorStyle_TitleBarHovered, IM_COL32(93,42,247, 255));
    
    imnodes::BeginNode(id);
        
        imnodes::BeginNodeTitleBar();
            ImGui::TextUnformatted("Flanger");
        imnodes::EndNodeTitleBar();

        imnodes::PushAttributeFlag(imnodes::AttributeFlags::AttributeFlags_EnableLinkDetachWithDragClick);
        imnodes::BeginInputAttribute(this->id+1);
        imnodes::EndInputAttribute();

        imnodes::BeginOutputAttribute(this->id+3);
        imnodes::EndOutputAttribute();
        imnodes::PopAttributeFlag();

        ImGui::DragFloat("Maximum Delay", &max_delay_time, 1.0f, 1.0f, MAX_POSSIBLE_DELAY_MS, "%.3f ms");
        // ImGui::DragFloat("LFO Frequency", &delay_frequency, 0.1f, 0.1f, 1.0f, "%.3f Hz");
        ImGui::DragFloat("Feedback Gain", &feedback_gain, 1.0f, -144.0f, -1.0f, "%.3f dB");

        // ImGui::Text("%.2f", ((static_cast<float>(max_delay_samples) * 0.5) - 1.0f) * (std::cos(2.0f * 3.1415927f * delay_frequency * internal_timer) + 1));

    imnodes::EndNode();
    ImGui::PopItemWidth();
    imnodes::PopColorStyle();
    imnodes::PopColorStyle();

}

void FlangerNode::ApplyFX(const float *in, float *out, size_t numFrames, AudioInfo info) {
    
    dsp::clamp(min_delay_time, 1.0f, 30.0f);
    dsp::clamp(max_delay_time, 1.0f, MAX_POSSIBLE_DELAY_MS);
    dsp::clamp(delay_frequency, 0.1f, 1.0f);
    delay_frequency = 0.001f;
    max_delay_samples = dsp::seconds_to_samples(max_delay_time, info.sample_rate);

    if (static_cast<size_t>(static_cast<float>(info.sample_rate) * (max_delay_time / 1000.0f) + 0.5) != delay_buf_size) {
        delay_buf_size = static_cast<size_t>(static_cast<float>(info.sample_rate) * (max_delay_time / 1000.0f) + 0.5);
        if (delay_buf != nullptr) {
            delete[] delay_buf;
        }
        delay_buf = new float[delay_buf_size];
        write_ptr = 0;
    }

    if (internal_info != info) {

        if (delay_buf != nullptr) {
            delete[] delay_buf;
        }

        delay_buf_size = static_cast<size_t>(static_cast<float>(info.sample_rate) * (MAX_DELAY_DURATION / 1000.0f) + 0.5);
        delay_buf = new float[delay_buf_size];

        internal_timer = 0.0f;
        internal_info = info;
        
    }

    for (size_t i = 0; i < numFrames; i++) {

        float lfo = ((static_cast<float>(max_delay_samples) * 0.5) - 1.0f) * (std::cos(2.0f * 3.1415927f * delay_frequency * internal_timer)+1);
        size_t delay_start = static_cast<size_t>(lfo+0.5f);
        size_t delay_end = static_cast<size_t>(lfo);
        float delay_fraction = lfo - std::floor(lfo);

        float y0 = delay_buf[(write_ptr - delay_start + delay_buf_size) % delay_buf_size];
        float y1 = delay_buf[(write_ptr - delay_end + delay_buf_size) % delay_buf_size];
        // dx is 1 unit wide

        float interpolated = ((y1 - y0) * (1.0f-delay_fraction)) + y0;

        delay_buf[write_ptr] = in[i];
        write_ptr = (write_ptr + 1) % delay_buf_size;
    
        out[i] = in[i] + interpolated;

        internal_timer += static_cast<float>(i) / static_cast<float>(info.sample_rate);
        internal_timer = std::fmod(internal_timer, 1.0f/delay_frequency);
    }
    
}