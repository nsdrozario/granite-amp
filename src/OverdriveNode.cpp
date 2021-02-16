#include <OverdriveNode.hpp>
#include <internal_dsp.hpp>

using namespace guitar_amp;

void OverdriveNode::showGui() {

    ImGui::PushItemWidth(100);
    imnodes::PushColorStyle(imnodes::ColorStyle_TitleBar, IM_COL32(201, 4, 126, 100));
    imnodes::PushColorStyle(imnodes::ColorStyle_TitleBarSelected, IM_COL32(201, 4, 126, 255));
    imnodes::PushColorStyle(imnodes::ColorStyle_TitleBarHovered, IM_COL32(201, 4, 126, 255));

    imnodes::BeginNode(this->id);

        imnodes::BeginNodeTitleBar();
            ImGui::TextUnformatted("Overdrive");
        imnodes::EndNodeTitleBar();
        imnodes::PushAttributeFlag(imnodes::AttributeFlags::AttributeFlags_EnableLinkDetachWithDragClick);
        imnodes::BeginInputAttribute(this->id+1);
        imnodes::EndInputAttribute();

        ImGui::DragFloat("Gain coefficient", &(this->gain_coefficient), 1.0, 0.0, 10.0, "%.3f");
        ImGui::DragFloat("Threshold", &(this->normalized_threshold), 0.01, 0, 1, "%.3f");
        ImGui::DragFloat("Low pass frequency", &(this->lpf_cutoff), 1, 0, 21000, "%.3f");
        ImGui::DragFloat("High pass frequency", &(this->hpf_cutoff), 1, 0, 21000, "%.3f");

        imnodes::BeginOutputAttribute(this->id+3);
        imnodes::EndOutputAttribute();
        imnodes::PopAttributeFlag();
    imnodes::EndNode();

    ImGui::PopItemWidth();
    imnodes::PopColorStyle();
    imnodes::PopColorStyle();

}

void OverdriveNode::ApplyFX(const kfr::univector<float> &in, kfr::univector<float> &out, size_t numFrames) {

    
    // apply hpf to out before hard clipping
    kfr::biquad_params<float> highpass_config[] = { kfr::biquad_highpass(this->hpf_cutoff, 1.0f) };
    kfr::univector<float> output_buf (in);
    kfr::univector<float> lpf_result = kfr::biquad(highpass_config, in);
    output_buf = lpf_result;
    
    // hard clip
    output_buf *= this->gain_coefficient;
    guitar_amp::dsp::hardclip(output_buf, output_buf, this->normalized_threshold, numFrames);

    // now apply lpf to out after hard clipping
    kfr::biquad_params<float> lowpass_config[] = { kfr::biquad_lowpass(this->lpf_cutoff, 1.0f) };
    out = kfr::biquad(lowpass_config, output_buf);

}