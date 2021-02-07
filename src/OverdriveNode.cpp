#include <OverdriveNode.hpp>
#include <internal_dsp.hpp>

using namespace guitar_amp;

void OverdriveNode::showGui() {

    imnodes::BeginNode(this->id);
        
        imnodes::BeginInputAttribute(this->id+1);
        imnodes::EndInputAttribute();

        ImGui::DragFloat("Gain coefficient", &(this->gain_coefficient), 1.0, 0.0, 10.0, "%.3f");
        ImGui::DragFloat("Threshold", &(this->normalized_threshold), 0.01, 0, 1, "%.3f");
        ImGui::DragFloat("Low pass frequency", &(this->lpf_cutoff), 1, 0, 21000, "%.3f");
        ImGui::DragFloat("High pass frequency", &(this->hpf_cutoff), 1, 0, 21000, "%.3f");

        imnodes::BeginOutputAttribute(this->id+3);
        imnodes::EndInputAttribute();

    imnodes::EndNode();
    
}

void OverdriveNode::ApplyFX(float *in, float *out, size_t numFrames) {

    // apply lpf to out before hard clipping

    dsp::hardclip(in, out, this->normalized_threshold, numFrames);

    // now apply hpf to out after hard clipping

}