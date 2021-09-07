#include <CabSimNode.hpp>
#include <iostream>
#include <imknob.hpp>
using namespace guitar_amp;

CabSimNode::CabSimNode(int id, const AudioInfo current_audio_info) : MiddleNode(id, current_audio_info) {

    ma_lpf2_config lpf_cfg = ma_lpf2_config_init(ma_format_f32, 1, current_audio_info.sample_rate, lpf_freq, lpf_q);
    ma_hpf2_config hpf_cfg = ma_hpf2_config_init(ma_format_f32, 1, current_audio_info.sample_rate, hpf_freq, hpf_q);
    ma_peak2_config presence_config = ma_peak2_config_init(ma_format_f32, 1, current_audio_info.sample_rate, presence_magnitude, presence_q, presence_freq);

    if (ma_lpf2_init(&lpf_cfg, &lpf) != MA_SUCCESS) {
        std::cerr << "error in CabSimNode constructor, lpf" << std::endl;
    }
    
    if (ma_hpf2_init(&hpf_cfg, &hpf) != MA_SUCCESS) {
        std::cerr << "error in CabSimNode constructor, hpf" << std::endl;
    }

    if (ma_peak2_init(&presence_config, &presence) != MA_SUCCESS) {
        std::cerr << "error in CabSimNode constructor, presence filter" << std::endl;

    }

    ma_peak2_config mid_config = ma_peak2_config_init(ma_format_f32, 1, current_audio_info.sample_rate, mid_scoop_magnitude, mid_scoop_q, mid_scoop_freq);

    if (ma_peak2_init(&mid_config, &mid_scoop) != MA_SUCCESS) {
        std::cerr << "error in CabSimNode constructor, mid filter" << std::endl;

    }
    
    ma_peak2_config low_mid_config = ma_peak2_config_init(ma_format_f32, 1, current_audio_info.sample_rate, low_mids_boost_magnitude, low_mids_boost_q, low_mids_boost_freq);

    if (ma_peak2_init(&low_mid_config, &low_mids_boost) != MA_SUCCESS) {
        std::cerr << "error in CabSimNode constructor, low mid filter" << std::endl;
    }

    delay.reinit(static_cast<size_t>(static_cast<float>(current_audio_info.sample_rate) * delay_time / 1000.0f), 0, 0);

}

CabSimNode::~CabSimNode() { }

void CabSimNode::reinit(CabSimSettings settings) {
    
}

void CabSimNode::showGui() {
    ImGui::PushItemWidth(100);
    ImNodes::PushColorStyle(ImNodesCol_TitleBar, IM_COL32(170,110,220, 100));
    ImNodes::PushColorStyle(ImNodesCol_TitleBarSelected, IM_COL32(170,110,220, 255));
    ImNodes::PushColorStyle(ImNodesCol_TitleBarHovered, IM_COL32(170,110,220, 255));
    
    ImNodes::BeginNode(id);
        
        ImNodes::BeginNodeTitleBar();
            ImGui::TextUnformatted("Cabinet Simulation");
        ImNodes::EndNodeTitleBar();
        ImNodes::PushAttributeFlag(ImNodesAttributeFlags_EnableLinkDetachWithDragClick);
        ImNodes::BeginInputAttribute(this->id+1);
        ImNodes::EndInputAttribute();

        ImNodes::BeginOutputAttribute(this->id+3);
        ImNodes::EndOutputAttribute();
        ImNodes::PopAttributeFlag();
        
        if (advancedMode) {
            if(ImGui::DragFloat("Delay Time", &delay_time, 1.0f, 1.0f, 10.0f, "%.3f ms")) {
                changed_delay = true;
            }
            
            bool lpf_freq_changed = ImGui::DragFloat("Treble Cut (Low Pass) Frequency", &lpf_freq, 500.0f, 1000.0f, 6000.0f, "%.3f Hz");
            bool lpf_q_changed = ImGui::DragFloat("Treble Cut (Low Pass) Q", &lpf_q, 0.1f, 1.0f, 2.0f, "%.3f");

            if (lpf_freq_changed || lpf_q_changed) {
                changed_lpf = true;
            }

            ImGui::NewLine();


            bool hpf_freq_changed = ImGui::DragFloat("Bass Cut (High Pass) Frequency", &hpf_freq, 10.0f, 10.0f, 120.0f, "%.3f Hz");
            bool hpf_q_changed = ImGui::DragFloat("Bass Cut (High Pass) Q", &hpf_q, 0.1f, 0.4f, 2.0f, "%.3f");

            if (hpf_freq_changed || hpf_q_changed) {
                changed_hpf = true;
            }

            ImGui::NewLine();

            bool low_mids_freq_changed = ImGui::DragFloat("Low Mid Scoop/Boost Frequency", &low_mids_boost_freq, 50.0f, 200.0f, 400.0f, "%.3f Hz");
            bool low_mids_magnitude_changed = ImGui::DragFloat("Low Mid Scoop/Boost Magnitude", &low_mids_boost_magnitude, 1.0f, -3.0f, 3.0f, "%.3f dB");
            bool low_mids_q_changed = ImGui::DragFloat("Low Mid Scoop/Boost Q", &low_mids_boost_q, 0.2f, 2.0f, 7.0f, "%.3f");

            if (low_mids_freq_changed || low_mids_magnitude_changed || low_mids_q_changed) {
                changed_low_mids_boost = true;
            }

            ImGui::NewLine();

            bool mid_freq_changed = ImGui::DragFloat("Mid Scoop/Boost Frequency", &mid_scoop_freq, 100.0f, 400.0f, 800.0f, "%.3f Hz");
            bool mid_magnitude_changed = ImGui::DragFloat("Mid Scoop/Boost Magnitude", &mid_scoop_magnitude, 1.0f, -6.0f, 2.0f, "%.3f dB");
            bool mid_q_changed = ImGui::DragFloat("Mid Scoop/Boost Q", &mid_scoop_q, 0.2f, 2.0f, 7.0f, "%.3f");

            if (mid_freq_changed || mid_magnitude_changed || mid_q_changed) {
                changed_mid_scoop = true;
            }

            ImGui::NewLine();

            bool presence_freq_changed = ImGui::DragFloat("Presence Frequency", &presence_freq, 50.0f, 900.0f, 1600.0f, "%.3f Hz");
            bool presence_magnitude_changed = ImGui::DragFloat("Presence Magnitude", &presence_magnitude, 1.0f, -6.0f, 3.0f, "%.3f dB");
            bool presence_q_changed = ImGui::DragFloat("Presence Q", &presence_q, 1.0f, 5.0f, 20.0f, "%.3f");

            if (presence_freq_changed || presence_magnitude_changed || presence_q_changed) {
                changed_presence = true;
            }
        } else {
            // use presets; there will be too many knobs
        }

    ImNodes::EndNode();

    ImGui::PopItemWidth();
    ImNodes::PopColorStyle();
    ImNodes::PopColorStyle();
}

void CabSimNode::ApplyFX(const float *in, float *out, size_t numFrames, AudioInfo info) {

    if (internal_info != info) {
        // if the sample rate changes everything has to be reinitalized
        ma_lpf2_config lpf_cfg = ma_lpf2_config_init(ma_format_f32, 1, info.sample_rate, lpf_freq, lpf_q);
        ma_hpf2_config hpf_cfg = ma_hpf2_config_init(ma_format_f32, 1, info.sample_rate, hpf_freq, hpf_q);
        ma_peak2_config presence_config = ma_peak2_config_init(ma_format_f32, 1, info.sample_rate, presence_magnitude, presence_q, presence_freq);

        if (ma_lpf2_init(&lpf_cfg, &lpf) != MA_SUCCESS) {
            std::cerr << "error in CabSimNode reinitialization, lpf" << std::endl;
        }
        
        if (ma_hpf2_init(&hpf_cfg, &hpf) != MA_SUCCESS) {
            std::cerr << "error in CabSimNode reinitialization, hpf" << std::endl;
        }

        if (ma_peak2_init(&presence_config, &presence) != MA_SUCCESS) {
            std::cerr << "error in CabSimNode reinitialization, presence filter" << std::endl;
        }

        ma_peak2_config mid_config = ma_peak2_config_init(ma_format_f32, 1, info.sample_rate, mid_scoop_magnitude, mid_scoop_q, mid_scoop_freq);

        if (ma_peak2_init(&mid_config, &mid_scoop) != MA_SUCCESS) {
            std::cerr << "error in CabSimNode reinitialization, mid filter" << std::endl;

        }
        
        ma_peak2_config low_mid_config = ma_peak2_config_init(ma_format_f32, 1, info.sample_rate, low_mids_boost_magnitude, low_mids_boost_q, low_mids_boost_freq);

        if (ma_peak2_init(&low_mid_config, &low_mids_boost) != MA_SUCCESS) {
            std::cerr << "error in CabSimNode reinitialization, low mid filter" << std::endl;
        }

        delay.reinit(static_cast<size_t>(static_cast<float>(info.sample_rate) * delay_time / 1000.0f), 0, 0);
    }    

    if (changed_delay) {
        delay.reinit(static_cast<size_t>(static_cast<float>(info.sample_rate) * delay_time / 1000.0f), 0, 0);
        changed_delay = false;
    }

    if (changed_lpf) {
        ma_lpf2_config lpf_cfg = ma_lpf2_config_init(ma_format_f32, 1, info.sample_rate, lpf_freq, lpf_q);
        if (ma_lpf2_init(&lpf_cfg, &lpf) != MA_SUCCESS) {
            std::cerr << "error in CabSimNode reinitialization, lpf" << std::endl;
        }
        changed_lpf = false;
    }

    if (changed_hpf) {
        ma_hpf2_config hpf_cfg = ma_hpf2_config_init(ma_format_f32, 1, info.sample_rate, hpf_freq, hpf_q);
        if (ma_hpf2_init(&hpf_cfg, &hpf) != MA_SUCCESS) {
            std::cerr << "error in CabSimNode reinitialization, hpf" << std::endl;
        }
        changed_hpf = false;
    }

    if (changed_presence) {
        ma_peak2_config presence_config = ma_peak2_config_init(ma_format_f32, 1, info.sample_rate, presence_magnitude, presence_q, presence_freq);
        if (ma_peak2_init(&presence_config, &presence) != MA_SUCCESS) {
            std::cerr << "error in CabSimNode reinitialization, presence filter" << std::endl;
        }
        changed_presence = false;
    }

    if (changed_mid_scoop) {
        ma_peak2_config mid_config = ma_peak2_config_init(ma_format_f32, 1, info.sample_rate, mid_scoop_magnitude, mid_scoop_q, mid_scoop_freq);
        if (ma_peak2_init(&mid_config, &mid_scoop) != MA_SUCCESS) {
            std::cerr << "error in CabSimNode reinitialization, mid filter" << std::endl;
        }
        changed_mid_scoop = false;
    }

    if (changed_low_mids_boost) {
        ma_peak2_config low_mid_config = ma_peak2_config_init(ma_format_f32, 1, info.sample_rate, low_mids_boost_magnitude, low_mids_boost_q, low_mids_boost_freq);
        if (ma_peak2_init(&low_mid_config, &low_mids_boost) != MA_SUCCESS) {
            std::cerr << "error in CabSimNode reinitialization, low mid filter" << std::endl;
        }
        changed_low_mids_boost = false;
    }


    /*
    // first apply the delay
    for (size_t i = 0; i < numFrames; i++) {
        float val = delay.get_read_ptr_value();
        out[i] = in[i] + val;
        delay.set_write_ptr_value(in[i]);
        delay.inc_read_ptr();
        delay.inc_write_ptr();
    }
    */

    // then apply all the biquad filters
    ma_lpf2_process_pcm_frames(&lpf, out, in, numFrames);
    ma_hpf2_process_pcm_frames(&hpf, out, out, numFrames);
    ma_peak2_process_pcm_frames(&low_mids_boost, out, out, numFrames);
    ma_peak2_process_pcm_frames(&mid_scoop, out, out, numFrames);
    ma_peak2_process_pcm_frames(&presence, out, out, numFrames);
    
}