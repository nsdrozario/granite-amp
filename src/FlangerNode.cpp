#include <FlangerNode.hpp>
#include <imknob.hpp>
#include <state.hpp>

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
    ImNodes::PushColorStyle(ImNodesCol_TitleBar, IM_COL32(93,42,247, 100));
    ImNodes::PushColorStyle(ImNodesCol_TitleBarSelected, IM_COL32(93,42,247, 255));
    ImNodes::PushColorStyle(ImNodesCol_TitleBarHovered, IM_COL32(93,42,247, 255));
    
    ImNodes::BeginNode(id);
        
        ImNodes::BeginNodeTitleBar();
            ImGui::TextUnformatted("Flanger");
        ImNodes::EndNodeTitleBar();

        ImNodes::PushAttributeFlag(ImNodesAttributeFlags_EnableLinkDetachWithDragClick);
        ImNodes::BeginInputAttribute(this->id+1);
        ImNodes::EndInputAttribute();

        ImNodes::BeginOutputAttribute(this->id+3);
        ImNodes::EndOutputAttribute();
        ImNodes::PopAttributeFlag();

        if (advancedMode) {
            ImGui::DragFloat("Maximum Delay", &max_delay_time, 1.0f, 1.0f, MAX_POSSIBLE_DELAY_MS, "%.3f ms");
            ImGui::DragFloat("LFO Frequency", &delay_frequency, 0.5f, 0.1f, 2.0f, "%.3f Hz");
            ImGui::DragFloat("Feedback Gain", &feedback_gain, 1.0f, -144.0f, -1.0f, "%.3f dB");
        } else {
            ImKnob::Knob("Maximum Delay", &max_delay_time, 1.0f, 1.0f, MAX_POSSIBLE_DELAY_MS, "%.1f ms", 18.0f, COLOR_KNOB_DARK, COLOR_KNOB_DARK_SELECTED);
            ImGui::SameLine();
            ImKnob::Knob("LFO Frequency", &delay_frequency, 1.0f, 0.5f, 2.0f, "%.1f Hz", 18.0f, COLOR_KNOB_DARK, COLOR_KNOB_DARK_SELECTED);
            ImGui::SameLine();
            ImKnob::Knob("Feedback", &feedback_gain, 1.0f, -60.0f, -1.0f, "%.0f", 18.0f, COLOR_KNOB_DARK, COLOR_KNOB_DARK_SELECTED);
        }
        // ImGui::Text("%.2f", ((static_cast<float>(max_delay_samples) * 0.5) - 1.0f) * (std::cos(2.0f * 3.1415927f * delay_frequency * internal_timer) + 1));

    ImNodes::EndNode();
    ImGui::PopItemWidth();
    ImNodes::PopColorStyle();
    ImNodes::PopColorStyle();

}

void FlangerNode::ApplyFX(const float *in, float *out, size_t numFrames, AudioInfo info) {
    
    dsp::clamp(min_delay_time, 1.0f, 30.0f);
    dsp::clamp(max_delay_time, 1.0f, MAX_POSSIBLE_DELAY_MS);

    max_delay_samples = dsp::seconds_to_samples(max_delay_time/1000.0f, info.sample_rate);

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

    if (max_delay_samples > delay_buf_size) {
        if (delay_buf != nullptr) {
            delete[] delay_buf;
        }
        delay_buf = new float[max_delay_samples + 1];
    }

    for (size_t i = 0; i < numFrames; i++) {

        float lfo_unscaled = std::cos(2.0f * 3.1415927f * delay_frequency * internal_timer / static_cast<float>(info.sample_rate)) + 1;
        float lfo = lfo_unscaled * ((static_cast<float>(max_delay_samples) * 0.5f) - 1.0f);
        float index = static_cast<float>(write_ptr) - lfo_unscaled;
        while (index < 0) {
            index += static_cast<float>(max_delay_samples);
        }
        float y0 = delay_buf[static_cast<size_t>(index)];
        float y1 = delay_buf[(static_cast<size_t>(index) + 1) % max_delay_samples];
        float fraction = index - std::floor(index);
        float interpolated = ((y1 - y0) * fraction) + y0;

        out[i] = in[i] + interpolated;

        delay_buf[write_ptr] = in[i] + (dsp::dbfs_to_f32(feedback_gain) * interpolated);
        write_ptr = (write_ptr + 1) % max_delay_samples;

        internal_timer += 1;
        internal_timer = std::fmod(internal_timer, static_cast<float>(info.sample_rate)/delay_frequency);
    }
    
}