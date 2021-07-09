#include <AnalyzerNode.hpp>
#include <implot.h>
#include <iostream>
using namespace guitar_amp;

AnalyzerNode::AnalyzerNode(int id, const AudioInfo current_audio_info) : MiddleNode(id, current_audio_info) {
    this->internal_fft.init(fftconvolver::NextPowerOf2(device.capture.internalPeriodSizeInFrames));
    this->fft_size = 0;
}

AnalyzerNode::~AnalyzerNode() { }

void AnalyzerNode::showGui() {
    imnodes::PushColorStyle(imnodes::ColorStyle_TitleBar, IM_COL32(0,171,142,255));
    imnodes::PushColorStyle(imnodes::ColorStyle_TitleBarSelected, IM_COL32(0,222,207,255));
    imnodes::PushColorStyle(imnodes::ColorStyle_TitleBarHovered, IM_COL32(0,222,207,255));

    imnodes::BeginNode(this->id);

        imnodes::BeginNodeTitleBar();
            ImGui::TextUnformatted("Spectral Analyzer");
        imnodes::EndNodeTitleBar();

        imnodes::PushAttributeFlag(imnodes::AttributeFlags::AttributeFlags_EnableLinkDetachWithDragClick);
        imnodes::BeginInputAttribute(this->id+1);
        imnodes::EndInputAttribute();
        ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(ImColor(255, 255, 102)));
        ImGui::Text("WARNING: The frequency analyzer fluctuates rapidly\nand may cause seizures in some people.\nIf you or any of your relatives have had\na history of epileptic conditions or seizures,\nplease consult a medical professional before using this software\nand DO NOT press the checkbox below.");
        ImGui::PopStyleColor();
        ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(ImColor(255, 153, 153)));
        ImGui::Text("This is not medical advice;\nplease consult a medical professional for medical advice.");
        ImGui::PopStyleColor();
        ImGui::Checkbox("I accept the warning and understand the risks.", &(this->accept_warning));
        
        if (this->accept_warning) {
            ImGui::Checkbox("Show Spectrum", &(this->showing_spectrum));
        } else {
            this->showing_spectrum = false;
        }

        if (this->showing_spectrum && this->accept_warning) {
            ImGui::BeginChildFrame(this->id, ImVec2(400,300));
            ImPlot::SetNextPlotLimitsX(0.0f, static_cast<double>(device.sampleRate/2));
            const double ticks[] = {0.0,0.05,0.2,0.5,1,4,8,20};
            ImPlot::SetNextPlotTicksX(ticks, 10);
            ImPlot::BeginPlot("FFT", "Frequency (kHz)", "Power (dBFS+60)", ImVec2(-1, 0), ImPlotFlags_None, ImPlotAxisFlags_LogScale);
            ImPlot::PlotLine("Power", this->freqs.data(), this->output.data(), this->fft_size/2);
            ImPlot::EndPlot();
            ImGui::EndChildFrame();
        }

        imnodes::BeginOutputAttribute(this->id+3);
        imnodes::EndOutputAttribute();
        imnodes::PopAttributeFlag();

    imnodes::EndNode();

    imnodes::PopColorStyle();
    imnodes::PopColorStyle();

}

void AnalyzerNode::ApplyFX(const float *in, float *out, size_t numFrames, AudioInfo info) {

    memcpy(out, in, numFrames * sizeof(float));
    // size_t needed_size = fftconvolver::NextPowerOf2(numFrames);
    size_t needed_size = 4096;
    fft_size = needed_size;
    if (needed_size != this->signal.size()) {
        this->internal_fft.init(needed_size);
        this->signal.resize(needed_size, 0.0f);
        this->real.resize(audiofft::AudioFFT::ComplexSize(needed_size), 0.0f);
        this->imaginary.resize(audiofft::AudioFFT::ComplexSize(needed_size), 0.0f);
        this->output.resize(needed_size);
    }

    for (size_t i = 0; i < signal.size(); i++) {
        // copy input buffer
        if (i < numFrames) {
            signal[i] = in[i];
        } else {
            signal[i] = 0.0f;
        }
        // apply hann window
        float sqrt_hann = sinf(M_PI * static_cast<float>(i) / static_cast<float>(signal.size()-1));
        signal[i] *= sqrt_hann * sqrt_hann;
    }


    this->internal_fft.fft(this->signal.data(), this->real.data(), this->imaginary.data());
    this->output_lock.lock();
    for (size_t i = 0; i < this->real.size(); i++) {
        output[i] = std::max(0.0f, dsp::f32_to_dbfs(sqrtf((this->real[i]*this->real[i]) + (this->imaginary[i]*this->imaginary[i]))) + 60.0f);
    }
    this->output_lock.unlock();

    if (this->freqs.size() != this->output.size()) {
        this->freqs.resize(this->output.size());
        for (size_t i = 0; i < this->output.size(); i++) {
            freqs[i] = ((static_cast<float>(i) * static_cast<float>(device.sampleRate)) / static_cast<float>(needed_size))/1000.0f;
        }
    }

}