#include <FlangerNode.hpp>
#include <imknob.hpp>
#include <state.hpp>

using namespace guitar_amp;

FlangerNode::FlangerNode(int id, const AudioInfo current_audio_info) : MiddleNode(id, current_audio_info) {

    if (current_audio_info.sample_rate == 0) {
        // don't allocate
    } else {
        size_t max_size = static_cast<size_t>(current_audio_info.sample_rate * (MAX_POSSIBLE_DELAY_MS/1000));
        rb.resize(max_size);
        rb.reset_data();
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

    avg_delay_time = init_table.get_or("DelayTimeMilliseconds", 1.0);
    delay_frequency = init_table.get_or("DelayFrequency", 1.0);
    feedback_gain = init_table.get_or("FeeedbackGain", -6.0);
    depth = init_table.get_or("Depth", 50);
    delay_range_time = init_table.get_or("DelayRange", 0.5);

    if (current_audio_info.sample_rate == 0) {
        // don't allocate
    } else {
        size_t max_size = static_cast<size_t>(current_audio_info.sample_rate * (MAX_POSSIBLE_DELAY_MS/1000));
        rb.resize(max_size);
        rb.reset_data();
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

    state["DelayTimeMilliseconds"] = avg_delay_time;
    state["DelayFrequency"] = delay_frequency;
    state["FeedbackGain"] = feedback_gain;
    state["Depth"] = depth;
    state["DelayRange"] = delay_range_time;
    
    out["state"] = state;
    
    return out;

}

void FlangerNode::luaInit(const sol::table &init_table) {
    avg_delay_time = init_table.get_or("DelayTimeMilliseconds", 1.0);
    delay_frequency = init_table.get_or("DelayFrequency", 1.0);
    feedback_gain = init_table.get_or("FeeedbackGain", -6.0);
    depth = init_table.get_or("Depth", 50.0);
    delay_range_time = init_table.get_or("DelayRange", 0.5);
}

FlangerNode::~FlangerNode() {

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
        
        float max_range = std::min(avg_delay_time, MAX_POSSIBLE_DELAY_MS - avg_delay_time);
        
        if (advancedMode) {
            if(ImGui::DragFloat("Average Delay", &avg_delay_time, 0.5f, 0.0f, MAX_POSSIBLE_DELAY_MS, "%.3f ms")) {
                changed_delay = true;
            } else {
                changed_delay = changed_delay || false;
            }
            if (ImGui::DragFloat("Delay Width", &delay_range_time, 0.2f, 0.0f, max_range, "%.3f ms")) {
                changed_delay = true;
            } else {
                changed_delay = changed_delay || false;
            }
            ImGui::DragFloat("LFO Frequency", &delay_frequency, 0.5f, 0.01f, 2.0f, "%.3f Hz");
            ImGui::DragFloat("Feedback Gain", &feedback_gain, 1.0f, -60.0f, -1.0f, "%.3f dB");
            ImGui::DragFloat("Depth", &depth, 1.0f, 0.0f, 100.0f, "%.0f%%");
        } else {
            if(ImKnob::Knob("Average Delay", &avg_delay_time, 1.0f, 0.0f, MAX_POSSIBLE_DELAY_MS, "%.1f ms", 18.0f, COLOR_KNOB_DARK, COLOR_KNOB_DARK_SELECTED)) {
                changed_delay = true;
            } else {
                changed_delay = changed_delay || false;
            }
            ImGui::SameLine();
            if (ImKnob::Knob("Delay Range", &delay_range_time, 0.2f, 0.0f, max_range, "%.1f ms", 18.0f, COLOR_KNOB_DARK, COLOR_KNOB_DARK_SELECTED)) {
                changed_delay = true;
            } else {
                changed_delay = changed_delay || false;
            }
            ImKnob::Knob("LFO Frequency ", &delay_frequency, 1.0f, 0.5f, 2.0f, "%.1f Hz", 18.0f, COLOR_KNOB_DARK, COLOR_KNOB_DARK_SELECTED);
            ImGui::SameLine();
            ImKnob::Knob("Feedback Gain", &feedback_gain, 1.0f, -60.0f, -1.0f, "%.0f dB", 18.0f, COLOR_KNOB_DARK, COLOR_KNOB_DARK_SELECTED);
            ImKnob::Knob("Depth", &depth, 1.0f, 0.0f, 100.0f, "%.0f%%", 18.0f, COLOR_KNOB_DARK, COLOR_KNOB_DARK_SELECTED);
        }
        // ImGui::Text("%.2f", ((static_cast<float>(max_delay_samples) * 0.5) - 1.0f) * (std::cos(2.0f * 3.1415927f * delay_frequency * internal_timer) + 1));

    ImNodes::EndNode();
    ImGui::PopItemWidth();
    ImNodes::PopColorStyle();
    ImNodes::PopColorStyle();

}

void FlangerNode::ApplyFX(const float *in, float *out, size_t numFrames, AudioInfo info) {
    
    // these can be local because we seem to reset them every frame anyway
    size_t avg_delay_samples = dsp::seconds_to_samples(avg_delay_time/1000.0f, info.sample_rate);
    size_t delay_range_samples = dsp::seconds_to_samples(delay_range_time/1000.0f, info.sample_rate);

    // check if we need reallocation
    if (internal_info != info) {

        size_t delay_buf_size = static_cast<size_t>(static_cast<float>(info.sample_rate) * (MAX_POSSIBLE_DELAY_MS / 1000.0f) + 0.5);
        rb.resize(delay_buf_size);
        rb.reset_data();

        // reset t variable
        internal_timer = 0.0f;
        internal_info = info;
    }

    for (size_t i = 0; i < numFrames; i++) {

        if (std::isfinite(in[i]) && std::abs(in[i]) <= 1) {
            rb.push(in[i]);
        } else {
            rb.push(0);
        }
        
        float depth_proportion = depth/100.0f;
        float delay = static_cast<float>(delay_range_samples) * std::sin(2.0 * M_PI * delay_frequency * internal_timer / internal_info.sample_rate) + static_cast<float>(avg_delay_samples);
        float shifted = rb.read_tap_lerp(delay);

        if (!std::isfinite(shifted) || std::abs(shifted) >= 1) {
            shifted = in[i];
        }

        out[i] = (depth_proportion * in[i]) + ((1-depth_proportion) * shifted);

        // t goes on
        internal_timer += 1;
        internal_timer = std::fmod(internal_timer, static_cast<float>(info.sample_rate));
    }
    
}