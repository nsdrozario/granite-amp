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

FlangerNode::FlangerNode(int id, const AudioInfo current_audio_info, const sol::table &init_table) : MiddleNode(id, current_audio_info) {

    /*
        Example info:

        {
            ["DelayTimeMilliseconds"]=1,
            ["DelayFrequency"]=1,
            ["FeedbackGain"]=-6
        }
    */

    max_delay_time = init_table.get_or("DelayTimeMilliseconds", 1.0);
    delay_frequency = init_table.get_or("DelayFrequency", 1.0);
    feedback_gain = init_table.get_or("FeeedbackGain", -6.0);
    depth = init_table.get_or("Depth", 50);

    if (current_audio_info.sample_rate == 0) {
        // don't allocate
    } else {
        delay_buf_size = static_cast<size_t>(static_cast<float>(current_audio_info.sample_rate) * (MAX_DELAY_DURATION / 1000.0f) + 0.5);
        delay_buf = new float[delay_buf_size];
    }

}

sol::table FlangerNode::serializeLua() {
    /*
        Example info:

        {
            ["DelayTimeMilliseconds"]=1,
            ["DelayFrequency"]=1,
            ["Depth"]=50,
            ["FeedbackGain"]=-6
        }
    */
    sol::table out;
    sol::table state;
    out["type"] = "Flanger";

    state["DelayTimeMilliseconds"] = max_delay_time;
    state["DelayFrequency"] = delay_frequency;
    state["FeedbackGain"] = feedback_gain;
    state["Depth"] = depth;
    
    out["state"] = state;
    
    return out;

}

void FlangerNode::luaInit(const sol::table &init_table) {
    max_delay_time = init_table.get_or("DelayTimeMilliseconds", 1.0);
    delay_frequency = init_table.get_or("DelayFrequency", 1.0);
    feedback_gain = init_table.get_or("FeeedbackGain", -6.0);
    depth = init_table.get_or("Depth", 50.0);
    // applyfx should take care of the reallocation
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
        ImNodes::BeginInputAttribute(this->id+1, ImNodesPinShape_TriangleFilled);
        ImNodes::EndInputAttribute();

        ImNodes::BeginOutputAttribute(this->id+3, ImNodesPinShape_TriangleFilled);
        ImNodes::EndOutputAttribute();
        ImNodes::PopAttributeFlag();

        if (advancedMode) {
            if(ImGui::DragFloat("Minimum Delay", &min_delay_time, 1.0f, 0.0f, max_delay_time, "%.3f ms")) {
                changed_delay = true;
            } else {
                changed_delay = false;
            }
            if (ImGui::DragFloat("Maximum Delay", &max_delay_time, 1.0f, 1.0f, MAX_POSSIBLE_DELAY_MS - 1.0f, "%.3f ms")) {
                changed_delay = true;
            } else {
                changed_delay = false;
            }
            ImGui::DragFloat("LFO Frequency", &delay_frequency, 0.5f, 0.1f, 2.0f, "%.3f Hz");
            ImGui::DragFloat("Feedback Gain", &feedback_gain, 1.0f, -144.0f, -1.0f, "%.3f dB");
            ImGui::DragFloat("Wet/Dry Ratio", &depth, 1.0f, 0.0f, 100.0f, "%.0f%%");
        } else {
            if(ImKnob::Knob("Minimum Delay", &min_delay_time, 1.0f, 1.0f, max_delay_time, "%.1f ms", 18.0f, COLOR_KNOB_DARK, COLOR_KNOB_DARK_SELECTED)) {
                internal_timer = 0;
            }
            ImGui::SameLine();
            ImKnob::Knob("Maximum Delay", &max_delay_time, 1.0f, 1.0f, MAX_POSSIBLE_DELAY_MS - 1.0f, "%.1f ms", 18.0f, COLOR_KNOB_DARK, COLOR_KNOB_DARK_SELECTED);
            ImKnob::Knob("LFO Frequency", &delay_frequency, 1.0f, 0.5f, 2.0f, "%.1f Hz", 18.0f, COLOR_KNOB_DARK, COLOR_KNOB_DARK_SELECTED);
            ImGui::SameLine();
            ImKnob::Knob("Feedback Gain", &feedback_gain, 1.0f, -60.0f, -1.0f, "%.0f dB", 18.0f, COLOR_KNOB_DARK, COLOR_KNOB_DARK_SELECTED);
            ImKnob::Knob("Wet/Dry Ratio", &depth, 1.0f, 0.0f, 100.0f, "%.0f%%", 18.0f, COLOR_KNOB_DARK, COLOR_KNOB_DARK_SELECTED);
        }
        // ImGui::Text("%.2f", ((static_cast<float>(max_delay_samples) * 0.5) - 1.0f) * (std::cos(2.0f * 3.1415927f * delay_frequency * internal_timer) + 1));

    ImNodes::EndNode();
    ImGui::PopItemWidth();
    ImNodes::PopColorStyle();
    ImNodes::PopColorStyle();

}

void FlangerNode::ApplyFX(const float *in, float *out, size_t numFrames, AudioInfo info) {
    
    max_delay_time = dsp::clamp(max_delay_time, 1.0f, MAX_POSSIBLE_DELAY_MS);
    min_delay_time = dsp::clamp(min_delay_time, 1.0f, max_delay_time-1.0f);
    
    max_delay_samples = dsp::seconds_to_samples(max_delay_time/1000.0f, info.sample_rate);
    min_delay_samples = dsp::seconds_to_samples(min_delay_time/1000.0f, info.sample_rate);

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

        delay_buf_size = static_cast<size_t>(static_cast<float>(info.sample_rate) * (MAX_POSSIBLE_DELAY_MS / 1000.0f) + 0.5);
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

        float lfo_unscaled = std::cos(2.0f * 3.1415927f * delay_frequency * internal_timer / static_cast<float>(info.sample_rate));
        float lfo = lfo_unscaled * ((static_cast<float>(max_delay_samples - min_delay_samples) * 0.5f));
        lfo += static_cast<float>(max_delay_samples+min_delay_samples) * 0.5;
        float index = static_cast<float>(write_ptr) - lfo_unscaled;
        while (index < 0) {
            index += static_cast<float>(max_delay_samples);
        }
        
        if (!std::isfinite(delay_buf[static_cast<size_t>(index)])) {
            delay_buf[static_cast<size_t>(index)] = 0;
        }

        if (!std::isfinite(delay_buf[(static_cast<size_t>(index) + 1) % max_delay_samples])) {
            delay_buf[(static_cast<size_t>(index) + 1) % max_delay_samples] = 0;
        }

        if (std::abs(delay_buf[static_cast<size_t>(index)]) >= 1 ) {
            delay_buf[static_cast<size_t>(index)] = 0;
        }

        if (std::abs(delay_buf[(static_cast<size_t>(index) + 1) % max_delay_samples]) >= 1) {
            delay_buf[(static_cast<size_t>(index) + 1) % max_delay_samples] = 0;
        }

        float y0 = delay_buf[static_cast<size_t>(index)];
        float y1 = delay_buf[(static_cast<size_t>(index) + 1) % max_delay_samples];
        
        float fraction = index - std::floor(index);
        float interpolated = ((y1 - y0) * fraction) + y0;

        if (!changed_delay) {
            delay_buf[write_ptr] = in[i] + (dsp::dbfs_to_f32(feedback_gain) * interpolated);
            out[i] = ((1-(depth/100)) * in[i]) + (interpolated * (depth/100));
        } else {
            out[i] = 0;
            delay_buf[write_ptr] = 0;
        }

        write_ptr = (write_ptr + 1) % max_delay_samples;

        internal_timer += 1;
        internal_timer = std::fmod(internal_timer, static_cast<float>(info.sample_rate)/delay_frequency);
    }
    
}