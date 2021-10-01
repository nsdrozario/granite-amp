#include <CabSimNode.hpp>
#include <iostream>
#include <imknob.hpp>
using namespace guitar_amp;

CabSimNode::CabSimNode(int id, const AudioInfo info) : MiddleNode(id, info) {

    lpf.set_coefficients(
        mindsp::filter::low_pass_filter(
            lpf_freq,
            info.sample_rate,
            2.0f
        )
    );

    hpf.set_coefficients(
        mindsp::filter::high_pass_filter(
            hpf_freq,
            info.sample_rate,
            2.0f
        )
    );

    low_mid.set_coefficients(
        mindsp::filter::peak_filter(
            low_mids_boost_freq,
            info.sample_rate,
            low_mids_boost_q,
            low_mids_boost_magnitude
        )
    );

    mid.set_coefficients(
        mindsp::filter::peak_filter(
            mid_scoop_freq,
            info.sample_rate,
            mid_scoop_q,
            mid_scoop_magnitude
        )
    );

    presence.set_coefficients(
        mindsp::filter::peak_filter(
            presence_freq,
            info.sample_rate,
            presence_q,
            presence_magnitude
        )
    );

   
    delay.reinit(static_cast<size_t>(static_cast<float>(info.sample_rate) * delay_time / 1000.0f), 0, 0);

}

CabSimNode::CabSimNode(int id, const AudioInfo info, const sol::table &init_table) : MiddleNode(id, info) {
    
    /*
        Example:
        {
            ["HPF"] = 60,
            ["LPF"] = 12000,
            ["LowMid"] = {
                ["freq"] = 250,
                ["q"] = 4,
                ["gain"] = 6
            },
            ["Mid"] = {
                ["freq"] = 500,
                ["q"] = 4,
                ["gain"] = -12
            },
            ["Presence"] = {
                ["freq"] = 1250,
                ["q"] = 12,
                ["gain"] = -6
            }
        }
    */

    lpf_freq = init_table.get_or("LPF", 12000.0);
    hpf_freq = init_table.get_or("HPF", 60.0);

    if (init_table.get<sol::table>("LowMid")) {
        low_mids_boost_freq = init_table.get<sol::table>("LowMid").get_or("freq", 250.0);
        low_mids_boost_magnitude = init_table.get<sol::table>("LowMid").get_or("gain", 6.0);
        low_mids_boost_q = init_table.get<sol::table>("LowMid").get_or("q", 4.0);
    }

    if (init_table["Mid"]) {
        mid_scoop_freq = init_table.get<sol::table>("Mid").get_or("freq", 500.0);
        mid_scoop_magnitude = init_table.get<sol::table>("Mid").get_or("gain", -12.0);
        mid_scoop_q = init_table.get<sol::table>("Mid").get_or("q", 4.0);
    }

    if (init_table["Presence"]) {
        presence_freq = init_table.get<sol::table>("Presence").get_or("freq", 1250.0);
        presence_magnitude = init_table.get<sol::table>("Presence").get_or("gain", -6.0);
        presence_q = init_table.get<sol::table>("Presence").get_or("q", 12.0);
    }

    lpf.set_coefficients(
        mindsp::filter::low_pass_filter(
            lpf_freq,
            info.sample_rate,
            2.0f
        )
    );

    hpf.set_coefficients(
        mindsp::filter::high_pass_filter(
            hpf_freq,
            info.sample_rate,
            2.0f
        )
    );

    low_mid.set_coefficients(
        mindsp::filter::peak_filter(
            low_mids_boost_freq,
            info.sample_rate,
            low_mids_boost_q,
            low_mids_boost_magnitude
        )
    );

    mid.set_coefficients(
        mindsp::filter::peak_filter(
            mid_scoop_freq,
            info.sample_rate,
            mid_scoop_q,
            mid_scoop_magnitude
        )
    );

    presence.set_coefficients(
        mindsp::filter::peak_filter(
            presence_freq,
            info.sample_rate,
            presence_q,
            presence_magnitude
        )
    );

}

void CabSimNode::luaInit(const sol::table &init_table) {
    lpf_freq = init_table.get_or("LPF", 12000.0);
    hpf_freq = init_table.get_or("HPF", 60.0);

    if (init_table.get<sol::table>("LowMid")) {
        low_mids_boost_freq = init_table.get<sol::table>("LowMid").get_or("freq", 250.0);
        low_mids_boost_magnitude = init_table.get<sol::table>("LowMid").get_or("gain", 6.0);
        low_mids_boost_q = init_table.get<sol::table>("LowMid").get_or("q", 4.0);
    }

    if (init_table["Mid"]) {
        mid_scoop_freq = init_table.get<sol::table>("Mid").get_or("freq", 500.0);
        mid_scoop_magnitude = init_table.get<sol::table>("Mid").get_or("gain", -12.0);
        mid_scoop_q = init_table.get<sol::table>("Mid").get_or("q", 4.0);
    }

    if (init_table["Presence"]) {
        presence_freq = init_table.get<sol::table>("Presence").get_or("freq", 1250.0);
        presence_magnitude = init_table.get<sol::table>("Presence").get_or("gain", -6.0);
        presence_q = init_table.get<sol::table>("Presence").get_or("q", 12.0);
    }

    lpf.set_coefficients(
        mindsp::filter::low_pass_filter(
            lpf_freq,
            internal_info.sample_rate,
            2.0f
        )
    );

    hpf.set_coefficients(
        mindsp::filter::high_pass_filter(
            hpf_freq,
            internal_info.sample_rate,
            2.0f
        )
    );

    low_mid.set_coefficients(
        mindsp::filter::peak_filter(
            low_mids_boost_freq,
            internal_info.sample_rate,
            low_mids_boost_q,
            low_mids_boost_magnitude
        )
    );

    mid.set_coefficients(
        mindsp::filter::peak_filter(
            mid_scoop_freq,
            internal_info.sample_rate,
            mid_scoop_q,
            mid_scoop_magnitude
        )
    );

    presence.set_coefficients(
        mindsp::filter::peak_filter(
            presence_freq,
            internal_info.sample_rate,
            presence_q,
            presence_magnitude
        )
    );
}

sol::table CabSimNode::serializeLua() {
    /*
        Example:
        {
            ["HPF"] = 60,
            ["LPF"] = 12000,
            ["LowMid"] = {
                ["freq"] = 250,
                ["q"] = 4,
                ["gain"] = 6
            },
            ["Mid"] = {
                ["freq"] = 500,
                ["q"] = 4,
                ["gain"] = -12
            },
            ["Presence"] = {
                ["freq"] = 1250,
                ["q"] = 12,
                ["gain"] = -6
            }
        }
    */

    sol::table out;
    sol::table state;
    sol::table low_mid_config;
    sol::table mid_config;
    sol::table presence_config;

    out["type"] = "CabSim";
    state["HPF"] = hpf_freq;
    state["LPF"] = lpf_freq;

    low_mid_config["freq"] = low_mids_boost_freq;
    low_mid_config["q"] = low_mids_boost_q;
    low_mid_config["gain"] = low_mids_boost_magnitude;

    mid_config["freq"] = mid_scoop_freq;
    mid_config["q"] = mid_scoop_q;
    mid_config["gain"] = mid_scoop_magnitude;

    presence_config["freq"] = presence_freq;
    presence_config["q"] = mid_scoop_q;
    presence_config["gain"] = mid_scoop_magnitude;

    state["LowMid"] = low_mid_config;
    state["Mid"] = mid_config;
    state["Presence"] = presence_config;

    out["state"] = state;

    return out;

}

CabSimNode::~CabSimNode() { }

void CabSimNode::reinit(CabSimSettings settings) {
    
    lpf_freq = settings.lpf.freq;
    lpf_q = settings.lpf.q;
    
    hpf_freq = settings.hpf.freq;
    hpf_q = settings.hpf.q;
    
    low_mids_boost_freq = settings.lowmid.freq;
    low_mids_boost_q = settings.lowmid.q;
    low_mids_boost_magnitude = settings.lowmid.gain_db;

    mid_scoop_freq = settings.mid.freq;
    mid_scoop_q = settings.mid.q;
    mid_scoop_magnitude = settings.mid.gain_db;

    presence_freq = settings.presence.freq;
    presence_q = settings.presence.q;
    presence_magnitude = settings.presence.gain_db;

    lpf.set_coefficients(
        mindsp::filter::low_pass_filter(
            lpf_freq,
            device.sampleRate,
            2.0f
        )
    );

    hpf.set_coefficients(
        mindsp::filter::high_pass_filter(
            hpf_freq,
            device.sampleRate,
            2.0f
        )
    );

    low_mid.set_coefficients(
        mindsp::filter::peak_filter(
            low_mids_boost_freq,
            device.sampleRate,
            low_mids_boost_q,
            low_mids_boost_magnitude
        )
    );

    mid.set_coefficients(
        mindsp::filter::peak_filter(
            mid_scoop_freq,
            device.sampleRate,
            mid_scoop_q,
            mid_scoop_magnitude
        )
    );

    presence.set_coefficients(
        mindsp::filter::peak_filter(
            presence_freq,
            device.sampleRate,
            presence_q,
            presence_magnitude
        )
    );

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

        lpf.set_coefficients(
            mindsp::filter::low_pass_filter(
                lpf_freq,
                info.sample_rate,
                2.0f
            )
        );

        hpf.set_coefficients(
            mindsp::filter::high_pass_filter(
                hpf_freq,
                info.sample_rate,
                2.0f
            )
        );

        low_mid.set_coefficients(
            mindsp::filter::peak_filter(
                low_mids_boost_freq,
                info.sample_rate,
                low_mids_boost_q,
                low_mids_boost_magnitude
            )
        );

        mid.set_coefficients(
            mindsp::filter::peak_filter(
                mid_scoop_freq,
                info.sample_rate,
                mid_scoop_q,
                mid_scoop_magnitude
            )
        );

        presence.set_coefficients(
            mindsp::filter::peak_filter(
                presence_freq,
                info.sample_rate,
                presence_q,
                presence_magnitude
            )
        );


        delay.reinit(static_cast<size_t>(static_cast<float>(info.sample_rate) * delay_time / 1000.0f), 0, 0);
    }    

    if (changed_delay) {
        delay.reinit(static_cast<size_t>(static_cast<float>(info.sample_rate) * delay_time / 1000.0f), 0, 0);
        changed_delay = false;
    }

    if (changed_lpf) {
        lpf.set_coefficients(
            mindsp::filter::low_pass_filter(
                lpf_freq,
                info.sample_rate,
                2.0f
            )
        );
        changed_lpf = false;
    }

    if (changed_hpf) {
        hpf.set_coefficients(
            mindsp::filter::high_pass_filter(
                hpf_freq,
                info.sample_rate,
                2.0f
            )
        );
        changed_hpf = false;
    }

    if (changed_presence) {
        presence.set_coefficients(
            mindsp::filter::peak_filter(
                presence_freq,
                info.sample_rate,
                presence_q,
                presence_magnitude
            )
        );
        changed_presence = false;
    }

    if (changed_mid_scoop) {
        mid.set_coefficients(
            mindsp::filter::peak_filter(
                mid_scoop_freq,
                info.sample_rate,
                mid_scoop_q,
                mid_scoop_magnitude
            )
        );
        changed_mid_scoop = false;
    }

    if (changed_low_mids_boost) {
        low_mid.set_coefficients(
            mindsp::filter::peak_filter(
                low_mids_boost_freq,
                info.sample_rate,
                low_mids_boost_q,
                low_mids_boost_magnitude
            )
        );
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
    lpf.apply(out,in,numFrames);
    hpf.apply(out,out,numFrames);
    low_mid.apply(out,out,numFrames);
    mid.apply(out,out,numFrames);
    presence.apply(out,out,numFrames);
}