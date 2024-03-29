#include <OscillatorNode.hpp>
using namespace guitar_amp;

OscillatorNode::OscillatorNode(int id, const AudioInfo current_audio_info) : MiddleNode(id, current_audio_info) {
    ma_waveform_config wave_gen_cfg = ma_waveform_config_init(
        ma_format_f32, 
        device.capture.channels, 
        device.sampleRate,
        ma_waveform_type_sine, 
        0.0f,
        440.0f
    );
    ma_waveform_init(&wave_gen_cfg, &(this->wav_gen));
}

OscillatorNode::OscillatorNode(int id, const AudioInfo current_audio_info, const sol::table &init_table) : MiddleNode(id, current_audio_info) {
    ma_waveform_config wave_gen_cfg = ma_waveform_config_init(
        ma_format_f32, 
        device.capture.channels, 
        device.sampleRate,
        ma_waveform_type_sine, 
        0.0f,
        440.0f
    );
    ma_waveform_init(&wave_gen_cfg, &(this->wav_gen));
}

void OscillatorNode::luaInit(const sol::table &init_table) {
    
}

OscillatorNode::~OscillatorNode() { };

void OscillatorNode::showGui() {
    
    ImGui::PushItemWidth(100);
    ImNodes::PushColorStyle(ImNodesCol_TitleBar, IM_COL32(232, 232, 232, 100));
    ImNodes::PushColorStyle(ImNodesCol_TitleBarSelected, IM_COL32(232, 232, 232, 255));
    ImNodes::PushColorStyle(ImNodesCol_TitleBarHovered, IM_COL32(232, 232, 232, 255));
    
    ImNodes::BeginNode(this->id);
        ImNodes::BeginNodeTitleBar();
            ImGui::TextColored(ImVec4(ImColor(18,18,18,255)), "Oscillator");
        ImNodes::EndNodeTitleBar();

        ImNodes::BeginInputAttribute(this->id+1, ImNodesPinShape_TriangleFilled);
        ImNodes::EndInputAttribute();
        ImNodes::BeginOutputAttribute(this->id+3, ImNodesPinShape_TriangleFilled);
        ImNodes::EndOutputAttribute();
        
        ImGui::Combo(
            "Wave type",
            &(this->wave_type),
            "Sine\0Square\0Triangle\0Sawtooth\0White noise\0"
        );

        ImGui::SliderFloat("Amplitude (dB)", &(this->amplitude), -60.0f, 0.0f, "%.3f dB");
        ImGui::SliderFloat("Frequency (Hz)", &(this->freq), 0.0f, 20000.0f, "%.3f Hz");

    ImNodes::EndNode();
    
}

sol::table OscillatorNode::serializeLua() {
    sol::table out;
    out["type"] = "Oscillator";
    out["state"] = sol::table();
    return out;
}

void OscillatorNode::ApplyFX(const float *in, float *out, size_t numFrames, AudioInfo info) {
    float linear_amplitude = dsp::dbfs_to_f32(this->amplitude);

    if (this->wav_gen.config.sampleRate != device.sampleRate) {
        ma_waveform_set_sample_rate(&(this->wav_gen), device.sampleRate);
    }
    if (std::abs(this->wav_gen.config.amplitude - linear_amplitude) > 0.001) {
        ma_waveform_set_amplitude(&(this->wav_gen), linear_amplitude);
    }
    if(std::abs(this->wav_gen.config.frequency - this->freq) > 0.001) {
        ma_waveform_set_frequency(&(this->wav_gen), this->freq);
    }
    if(this->wav_gen.config.type != this->wave_type) {
        ma_waveform_type t = ma_waveform_type_sine;
        switch(this->wave_type) {
            case 0:
                t = ma_waveform_type_sine;
            break;
            case 1:
                t = ma_waveform_type_square;
            break;
            case 2:
                t = ma_waveform_type_triangle;
            break;
            case 3:
                t = ma_waveform_type_sawtooth;
            break;
        }
        ma_waveform_set_type(&(this->wav_gen), t);
    }
    if (this->wave_type != 4) {
    ma_waveform_read_pcm_frames(&(this->wav_gen), out, numFrames);
    } else {
        for (size_t i = 0; i < numFrames; i++) {
            out[i] = (2.0f * (static_cast<float>(rand()) / RAND_MAX) - 0.5f) * linear_amplitude;
        }
    }
}