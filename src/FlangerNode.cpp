#include <FlangerNode.hpp>
using namespace guitar_amp;

FlangerNode::FlangerNode(int id, const AudioInfo current_audio_info) : MiddleNode(id, current_audio_info) {

    rb.reinit( 
        static_cast<size_t>(static_cast<float> (current_audio_info.sample_rate) * MAX_POSSIBLE_DELAY_MS / 1000.0f),
        0,
        0
    );

    min_delay_samples = dsp::seconds_to_samples(min_delay_time / 1000.0f, current_audio_info.sample_rate);
    max_delay_samples = dsp::seconds_to_samples(max_delay_time / 1000.0f, current_audio_info.sample_rate);

    rb.set_delay_no_resize(static_cast<float>(max_delay_samples-min_delay_samples)/2); // sin(0) = 0

}

FlangerNode::~FlangerNode() {

}

void FlangerNode::showGui() {

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

        ImGui::DragFloat("Maximum Delay", &max_delay_time, 1.0f, 1.0f, 30.0f, "%.3f ms");
        ImGui::DragFloat("LFO Frequency", &delay_frequency, 0.1f, 0.1f, 10.0f, "%.3f Hz");
        ImGui::DragFloat("Feedback Gain", &feedback_gain, 1.0f, -144.0f, -1.0f, "%.3f dB");

    imnodes::EndNode();

    imnodes::PopColorStyle();
    imnodes::PopColorStyle();

}

void FlangerNode::ApplyFX(const float *in, float *out, size_t numFrames, AudioInfo info) {
    
    std::clamp(min_delay_time, 1.0f, 30.0f);
    std::clamp(max_delay_time, 1.0f, 30.0f);
    std::clamp(delay_frequency, 0.1f, 10.0f);

    min_delay_samples = dsp::seconds_to_samples(min_delay_time / 1000.0f, info.sample_rate);
    max_delay_samples = dsp::seconds_to_samples(max_delay_time / 1000.0f, info.sample_rate);

    if (info != internal_info) {
        rb.reinit( 
        static_cast<size_t>(static_cast<float> (info.sample_rate) * MAX_POSSIBLE_DELAY_MS / 1000.0f),
        0,
        0
        );

        min_delay_samples = dsp::seconds_to_samples(min_delay_time / 1000.0f, info.sample_rate);
        max_delay_samples = dsp::seconds_to_samples(max_delay_time / 1000.0f, info.sample_rate);

        rb.set_delay_no_resize(static_cast<float>(max_delay_samples-min_delay_samples)/2); // sin(0) = 0
        internal_timer = 0.0f;
        internal_info = info;
    }

    // read first then apply the delay
    for (size_t i = 0; i < numFrames; i++) {
        out[i] = in[i] + rb.get_read_ptr_interpolated();
        rb.set_write_ptr_value( ( in[i]  ) * dsp::dbfs_to_f32(feedback_gain) );
        rb.inc_read_ptr( 
            (static_cast<float>(max_delay_samples)*0.5) * sin (2.0f * M_PI * delay_frequency * i / info.sample_rate)
            +
            (static_cast<float>(max_delay_samples)*0.5)
        );
        rb.inc_write_ptr();
    }

}