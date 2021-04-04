#include <AnalyzerNode.hpp>
#include <implot.h>
#include <iostream>
using namespace guitar_amp;

AnalyzerNode::AnalyzerNode(int id) : MiddleNode(id) {
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
        

        ImGui::BeginChildFrame(this->id, ImVec2(300,200));
        ImPlot::BeginPlot("FFT", "Frequency", "Power", ImVec2(-1, 0), ImPlotFlags_None, ImPlotAxisFlags_LogScale);
        ImPlot::PlotLine("Power", this->freqs.data(), this->output.data(), this->fft_size/2);
        ImPlot::EndPlot();
        // ImGui::EndPopup();
        ImGui::EndChildFrame();

        imnodes::BeginOutputAttribute(this->id+3);
        imnodes::EndOutputAttribute();
        imnodes::PopAttributeFlag();

    imnodes::EndNode();

    imnodes::PopColorStyle();
    imnodes::PopColorStyle();

}

void AnalyzerNode::ApplyFX(const float *in, float *out, size_t numFrames) {

    memcpy(out, in, numFrames * sizeof(float));
    size_t needed_size = fftconvolver::NextPowerOf2(numFrames);
    fft_size = needed_size;
    if (needed_size != this->signal.size()) {
        this->internal_fft.init(needed_size);
        this->signal.resize(needed_size, 0.0f);
        this->real.resize(audiofft::AudioFFT::ComplexSize(needed_size), 0.0f);
        this->imaginary.resize(audiofft::AudioFFT::ComplexSize(needed_size), 0.0f);
        this->output.resize(needed_size);
    }

    for (size_t i = 0; i < signal.size(); i++) {
        if (i < numFrames) {
            signal[i] = in[i];
        } else {
            signal[i] = 0.0f;
        }
    }

    this->internal_fft.fft(this->signal.data(), this->real.data(), this->imaginary.data());
    this->output_lock.lock();
    for (size_t i = 0; i < this->real.size(); i++) {
        output[i] = dsp::f32_to_dbfs(sqrtf((this->real[i]*this->real[i]) + (this->imaginary[i]*this->imaginary[i])));
    }
    this->output_lock.unlock();

    if (this->freqs.size() != this->output.size()) {
        this->freqs.resize(this->output.size());
        for (size_t i = 0; i < this->output.size(); i++) {
            freqs[i] = (static_cast<float>(i) * static_cast<float>(device.sampleRate)) / static_cast<float>(needed_size);
        }
    }

}