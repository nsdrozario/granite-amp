#include <ThreeBandEQ.hpp>
#include <state.hpp>
#include <imknob.hpp>

using namespace guitar_amp;

ThreeBandEQ::ThreeBandEQ(int id, const AudioInfo current_audio_info) : MiddleNode(id, current_audio_info) {

        low_shelf.set_coefficients(
            mindsp::filter::low_shelf(
                250.0f, 
                current_audio_info.sample_rate ? current_audio_info.sample_rate : 48000, 
                2.0f, 
                bass_gain
            )
        );

        high_shelf.set_coefficients(
            mindsp::filter::high_shelf(
                4000,
                current_audio_info.sample_rate ? current_audio_info.sample_rate : 48000,
                2.0f,
                treble_gain
            )
        );

        mid_range.set_coefficients(
            mindsp::filter::peak_filter(
                900,
                current_audio_info.sample_rate ? current_audio_info.sample_rate : 48000,
                0.5,
                mid_gain
            )
        );

}

ThreeBandEQ::ThreeBandEQ(int id, const AudioInfo current_audio_info, const sol::table &init_table) : MiddleNode(id, current_audio_info) {

        /*
            Example config:

            {
                ["BassGain"]=0,
                ["TrebleGain"]=0,
                ["MidGain"]=0
            }

        */

        low_shelf.set_coefficients(
            mindsp::filter::low_shelf(
                250, 
                current_audio_info.sample_rate ? current_audio_info.sample_rate : 48000, 
                2.0f, 
                init_table.get_or("BassGain", 0.0)
            )
        );

        high_shelf.set_coefficients(
            mindsp::filter::high_shelf(
                4000,
                current_audio_info.sample_rate ? current_audio_info.sample_rate : 48000,
                2.0f,
                init_table.get_or("TrebleGain", 0.0)
            )
        );

        mid_range.set_coefficients(
            mindsp::filter::peak_filter(
                900,
                current_audio_info.sample_rate ? current_audio_info.sample_rate : 48000,
                0.5,
                init_table.get_or("MidGain", 0.0)
            )
        );

}

ThreeBandEQ::~ThreeBandEQ() {

}

void ThreeBandEQ::luaInit(const sol::table &init_table) {
            low_shelf.set_coefficients(
            mindsp::filter::low_shelf(
                250, 
                internal_info.sample_rate ? internal_info.sample_rate : 48000, 
                2.0f, 
                init_table.get_or("BassGain", 0.0)
            )
        );

        high_shelf.set_coefficients(
            mindsp::filter::high_shelf(
                4000,
                internal_info.sample_rate ? internal_info.sample_rate : 48000,
                2.0f,
                init_table.get_or("TrebleGain", 0.0)
            )
        );

        mid_range.set_coefficients(
            mindsp::filter::peak_filter(
                900,
                internal_info.sample_rate ? internal_info.sample_rate : 48000,
                0.5,
                init_table.get_or("MidGain", 0.0)
            )
        );
}

void ThreeBandEQ::showGui() {
    ImGui::PushItemWidth(100);
    ImNodes::PushColorStyle(ImNodesCol_TitleBar, IM_COL32(240, 222, 29, 100));
    ImNodes::PushColorStyle(ImNodesCol_TitleBarSelected, IM_COL32(240, 222, 29, 255));
    ImNodes::PushColorStyle(ImNodesCol_TitleBarHovered, IM_COL32(240, 222, 29, 255));
    
    ImNodes::BeginNode(this->id);
        ImNodes::BeginNodeTitleBar();
            ImGui::TextColored(ImVec4(ImColor(18,18,18,255)), "Simple EQ");
        ImNodes::EndNodeTitleBar();

        ImNodes::BeginInputAttribute(this->id+1);
        ImNodes::EndInputAttribute();
        ImNodes::BeginOutputAttribute(this->id+3);
        ImNodes::EndOutputAttribute();
        
        if (advancedMode) {

        } else {
            
            if(ImKnob::Knob("Bass", &bass_gain, 1.0f, -40.0f, 20.0f, "%.0f dB", 24.0f, COLOR_KNOB_DARK, COLOR_KNOB_DARK_SELECTED)) {
                low_shelf.set_coefficients(
                    mindsp::filter::low_shelf(
                        250.0f, 
                        device.sampleRate ? device.sampleRate : 48000, 
                        2.0f, 
                        bass_gain
                    )
                );
            }
            
            ImGui::SameLine();
            
            if(ImKnob::Knob("Mid", &mid_gain, 1.0f, -40.0f, 20.0f, "%.0f dB", 24.0f, COLOR_KNOB_DARK, COLOR_KNOB_DARK_SELECTED)) {
                mid_range.set_coefficients(
                    mindsp::filter::peak_filter(
                        900,
                        device.sampleRate ? device.sampleRate : 48000,
                        0.5,
                        mid_gain
                    )
                );     
            }

            ImGui::SameLine();
            
            if(ImKnob::Knob("Treble", &treble_gain, 1.0f, -40.0f, 20.0f, "%.0f dB", 24.0f, COLOR_KNOB_DARK, COLOR_KNOB_DARK_SELECTED)) {
                high_shelf.set_coefficients(
                    mindsp::filter::high_shelf(
                        4000,
                        device.sampleRate ? device.sampleRate : 48000,
                        2.0f,
                        treble_gain
                    )
                );
            }

        }

    ImNodes::EndNode();
}

void ThreeBandEQ::ApplyFX(const float *in, float *out, size_t numFrames, AudioInfo info) {
    
    bool device_updated = internal_info != info;

    if (device_updated) {

        low_shelf.set_coefficients(
            mindsp::filter::low_shelf(
                250.0f, 
                info.sample_rate ? info.sample_rate : 48000, 
                2.0f, 
                bass_gain
            )
        );

        high_shelf.set_coefficients(
            mindsp::filter::high_shelf(
                4000,
                info.sample_rate ? info.sample_rate : 48000,
                2.0f,
                treble_gain
            )
        );

        mid_range.set_coefficients(
            mindsp::filter::peak_filter(
                900,
                info.sample_rate ? info.sample_rate : 48000,
                0.5,
                mid_gain
            )
        );

        if (device_updated) {
            internal_info = info;
        }
        
    }

    low_shelf.apply(out,in,numFrames);
    high_shelf.apply(out,out,numFrames);
    mid_range.apply(out,out,numFrames);

}