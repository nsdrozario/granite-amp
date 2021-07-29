#include <CabSimNode.hpp>
using namespace guitar_amp;

CabSimNode::CabSimNode(int id, const AudioInfo current_audio_info) : MiddleNode(id, current_audio_info) {

    ma_lpf2_config lpf_cfg = ma_lpf2_config_init(ma_format_f32, 1, current_audio_info.sample_rate, lpf_freq, lpf_q);
    ma_hpf2_config hpf_cfg = ma_hpf2_config_init(ma_format_f32, 1, current_audio_info.sample_rate, hpf_freq, hpf_q);
    ma_peak2_config peak_config = ma_peak2_config_init(ma_format_f32, 1, current_audio_info.sample_rate, presence_magnitude, presence_q, presence_freq);

    ma_lpf2_init(&lpf_cfg, &lpf);
    ma_hpf2_init(&hpf_cfg, &hpf);
    ma_peak2_init(&peak_config, &presence);

    peak_config.frequency = mid_scoop_freq;
    peak_config.gainDB = mid_scoop_magnitude;
    peak_config.q = mid_scoop_q;
    ma_peak2_init(&peak_config, &mid_scoop);
    
    peak_config.frequency = low_mids_boost_freq;
    peak_config.gainDB = low_mids_boost_magnitude;
    peak_config.q = low_mids_boost_q;
    ma_peak2_init(&peak_config, &mid_scoop);

    delay.reinit(static_cast<size_t>(static_cast<float>(current_audio_info.sample_rate) * delay_time / 1000.0f), 0, 0);

}

CabSimNode::~CabSimNode() { }

void CabSimNode::showGui() {
    ImGui::PushItemWidth(100);
    imnodes::PushColorStyle(imnodes::ColorStyle_TitleBar, IM_COL32(170,110,220, 100));
    imnodes::PushColorStyle(imnodes::ColorStyle_TitleBarSelected, IM_COL32(170,110,220, 255));
    imnodes::PushColorStyle(imnodes::ColorStyle_TitleBarHovered, IM_COL32(170,110,220, 255));
    
    imnodes::BeginNode(id);
        
        imnodes::BeginNodeTitleBar();
            ImGui::TextUnformatted("Cabinet Simulation");
        imnodes::EndNodeTitleBar();

        imnodes::PushAttributeFlag(imnodes::AttributeFlags::AttributeFlags_EnableLinkDetachWithDragClick);
        imnodes::BeginInputAttribute(this->id+1);
        imnodes::EndInputAttribute();

        ImGui::DragFloat("Delay Time", &delay_time, 1.0f, 1.0f, 10.0f, "%.3f ms");
        ImGui::DragFloat("Treble Cut (Low Pass) Frequency", &lpf_freq, 500.0f, 1000.0f, 6000.0f, "%.3f Hz");
        ImGui::DragFloat("Bass Cut (High Pass) Frequency", &hpf_freq, 10.0f, 10.0f, 120.0f, "%.3f Hz");
        ImGui::DragFloat("Low Mid Scoop/Boost Frequency", &low_mids_boost_freq, 50.0f, 200.0f, 400.0f, "%.3f Hz");
        ImGui::DragFloat("Mid Scoop/Boost Frequency", &mid_scoop_freq, 100.0f, 400.0f, 800.0f, "%.3f Hz");
        ImGui::DragFloat("Presence Frequency", &presence_freq, 50.0f, 900.0f, 1600.0f, "%.3f Hz");

        ImGui::NewLine();
        
        ImGui::DragFloat("Low Mid Scoop/Boost Magnitude", &low_mids_boost_magnitude, 1.0f, -3.0f, 3.0f, "%.3f dB");
        ImGui::DragFloat("Mid Scoop/Boost Magnitude", &mid_scoop_magnitude, 1.0f, -6.0f, 2.0f, "%.3f dB");
        ImGui::DragFloat("Presence Magnitude", &presence_magnitude, 1.0f, -6.0f, 3.0f, "%.3f dB");
        
        ImGui::NewLine();
        
        ImGui::DragFloat("Treble Cut (Low Pass) Q", &lpf_q, 0.1f, 1.0f, 2.0f, "%.3f");
        ImGui::DragFloat("Bass Cut (High Pass) Q", &hpf_q, 0.1f, 0.4f, 2.0f, "%.3f");
        ImGui::DragFloat("Low Mid Scoop/Boost Q", &low_mids_boost_q, 0.2f, 2.0f, 7.0f, "%.3f");
        ImGui::DragFloat("Mid Scoop/Boost Q", &mid_scoop_q, 0.2f, 2.0f, 7.0f, "%.3f");
        ImGui::DragFloat("Presence Q", &presence_q, 1.0f, 5.0f, 20.0f, "%.3f");

        imnodes::BeginOutputAttribute(this->id+3);
        imnodes::EndOutputAttribute();
        imnodes::PopAttributeFlag();
    imnodes::EndNode();

    ImGui::PopItemWidth();
    imnodes::PopColorStyle();
    imnodes::PopColorStyle();
}

void CabSimNode::ApplyFX(const float *in, float *out, size_t numFrames, AudioInfo info) {

    if (internal_info != info) {
        // if the sample rate changes everything has to be reinitalized
        ma_lpf2_config lpf_cfg = ma_lpf2_config_init(ma_format_f32, 1, info.sample_rate, lpf_freq, lpf_q);
        ma_hpf2_config hpf_cfg = ma_hpf2_config_init(ma_format_f32, 1, info.sample_rate, hpf_freq, hpf_q);
        ma_peak2_config peak_config = ma_peak2_config_init(ma_format_f32, 1, info.sample_rate, presence_magnitude, presence_q, presence_freq);

        ma_lpf2_init(&lpf_cfg, &lpf);
        ma_hpf2_init(&hpf_cfg, &hpf);
        ma_peak2_init(&peak_config, &presence);

        peak_config.frequency = mid_scoop_freq;
        peak_config.gainDB = mid_scoop_magnitude;
        peak_config.q = mid_scoop_q;
        ma_peak2_init(&peak_config, &mid_scoop);
        
        peak_config.frequency = low_mids_boost_freq;
        peak_config.gainDB = low_mids_boost_magnitude;
        peak_config.q = low_mids_boost_q;
        ma_peak2_init(&peak_config, &mid_scoop);

        delay.reinit(static_cast<size_t>(static_cast<float>(info.sample_rate) * delay_time / 1000.0f), 0, 0);
    }    

    // first apply the delay
    for (size_t i = 0; i < numFrames; i++) {
        float val = delay.get_read_ptr_value();
        out[i] = in[i] + val;
        delay.set_write_ptr_value(in[i]);
        delay.inc_read_ptr();
        delay.inc_write_ptr();
    }

    // then apply all the biquad filters
    ma_lpf2_process_pcm_frames(&lpf, out, out, numFrames);
    ma_hpf2_process_pcm_frames(&hpf, out, out, numFrames);
    ma_peak2_process_pcm_frames(&low_mids_boost, out, out, numFrames);
    ma_peak2_process_pcm_frames(&mid_scoop, out, out, numFrames);
    ma_peak2_process_pcm_frames(&presence, out, out, numFrames);
    
}