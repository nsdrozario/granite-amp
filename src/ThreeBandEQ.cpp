#include <ThreeBandEQ.hpp>
#include <state.hpp>
#include <imknob.hpp>

using namespace guitar_amp;

ThreeBandEQ::ThreeBandEQ(int id, const AudioInfo current_audio_info) : MiddleNode(id, current_audio_info) {
        hishelf_config = ma_hishelf2_config_init(
            ma_format_f32,
            1,
            current_audio_info.sample_rate,
            treble_gain,
            2.0,
            4000.0
        );

        loshelf_config = ma_loshelf2_config_init(
            ma_format_f32,
            1,
            current_audio_info.sample_rate,
            bass_gain,
            2.0,
            250.0
        );

        mid_config = ma_peak2_config_init(
            ma_format_f32,
            1,
            current_audio_info.sample_rate,
            mid_gain,
            0.5,
            900.0
        );

        ma_result hishelf_result = ma_hishelf2_init(&hishelf_config, &high_shelf);
        ma_result loshelf_result = ma_loshelf2_init(&loshelf_config, &low_shelf);
        ma_result mid_result = ma_peak2_init(&mid_config, &midrange);

        if (hishelf_result != MA_SUCCESS || loshelf_result != MA_SUCCESS || mid_result != MA_SUCCESS) {
            
        }
};

ThreeBandEQ::~ThreeBandEQ() {

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
            ImKnob::Knob("Bass", &bass_gain, 1.0f, -40.0f, 20.0f, "%.0f dB", 24.0f, COLOR_KNOB_DARK, COLOR_KNOB_DARK_SELECTED);
            ImGui::SameLine();
            ImKnob::Knob("Mid", &mid_gain, 1.0f, -40.0f, 20.0f, "%.0f dB", 24.0f, COLOR_KNOB_DARK, COLOR_KNOB_DARK_SELECTED);
            ImGui::SameLine();
            ImKnob::Knob("Treble", &treble_gain, 1.0f, -40.0f, 20.0f, "%.0f dB", 24.0f, COLOR_KNOB_DARK, COLOR_KNOB_DARK_SELECTED);
        }

    ImNodes::EndNode();
}

void ThreeBandEQ::ApplyFX(const float *in, float *out, size_t numFrames, AudioInfo info) {
    
    bool device_updated = internal_info != info;
    bool loshelf_changed = bass_gain != loshelf_config.gainDB;
    bool hishelf_changed = treble_gain != hishelf_config.gainDB;
    bool mid_changed = mid_gain != mid_config.gainDB;

    if (device_updated || loshelf_changed || hishelf_changed || mid_changed) {

        hishelf_config = ma_hishelf2_config_init(
            ma_format_f32,
            1,
            info.sample_rate,
            treble_gain,
            2.0,
            4000.0
        );

        loshelf_config = ma_loshelf2_config_init(
            ma_format_f32,
            1,
            info.sample_rate,
            bass_gain,
            2.0,
            250.0
        );

        mid_config = ma_peak2_config_init(
            ma_format_f32,
            1,
            info.sample_rate,
            mid_gain,
            0.5,
            900.0
        );

        ma_result hishelf_result = ma_hishelf2_init(&hishelf_config, &high_shelf);
        ma_result loshelf_result = ma_loshelf2_init(&loshelf_config, &low_shelf);
        ma_result mid_result = ma_peak2_init(&mid_config, &midrange);

        if (hishelf_result != MA_SUCCESS || loshelf_result != MA_SUCCESS || mid_result != MA_SUCCESS) {
            
        }

        if (device_updated) {
            internal_info = info;
        }
    }

    ma_loshelf2_process_pcm_frames(&low_shelf, out, in, numFrames);
    ma_hishelf2_process_pcm_frames(&high_shelf, out, out, numFrames);
    ma_peak2_process_pcm_frames(&midrange, out, out, numFrames);

}