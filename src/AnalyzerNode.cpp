#include <AnalyzerNode.hpp>
#include <implot.h>
#include <iostream>
using namespace guitar_amp;

AnalyzerNode::AnalyzerNode(int id, const AudioInfo current_audio_info) : MiddleNode(id, current_audio_info) {
    this->internal_fft.init(fftconvolver::NextPowerOf2(device.capture.internalPeriodSizeInFrames));
    this->fft_size = 0;
}

AnalyzerNode::AnalyzerNode(int id, const AudioInfo current_audio_info, const sol::table &init_table) : MiddleNode(id, current_audio_info) {
    this->internal_fft.init(fftconvolver::NextPowerOf2(device.capture.internalPeriodSizeInFrames));
    this->fft_size = 0;
}

void AnalyzerNode::luaInit(const sol::table &init_table) { }
sol::table AnalyzerNode::serializeLua() { 
    sol::table out;
    out["type"] = "Analyzer";
    out["state"] = sol::table();
}

AnalyzerNode::~AnalyzerNode() { }

void AnalyzerNode::showGui() {

    ImNodes::PushColorStyle(ImNodesCol_TitleBar, IM_COL32(0,171,142,255));
    ImNodes::PushColorStyle(ImNodesCol_TitleBarSelected, IM_COL32(0,222,207,255));
    ImNodes::PushColorStyle(ImNodesCol_TitleBarHovered, IM_COL32(0,222,207,255));

    ImNodes::BeginNode(this->id);

        ImNodes::BeginNodeTitleBar();
            ImGui::TextUnformatted("Spectral Analyzer");
        ImNodes::EndNodeTitleBar();

        ImNodes::PushAttributeFlag(ImNodesAttributeFlags_EnableLinkDetachWithDragClick);
        ImNodes::BeginInputAttribute(this->id+1);
        ImNodes::EndInputAttribute();
        ImNodes::BeginOutputAttribute(this->id+3);
        ImNodes::EndOutputAttribute();
        ImNodes::PopAttributeFlag();

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
            ImGui::Checkbox("Frequency Domain", &freqDomain);
            ImGui::BeginChildFrame(this->id, ImVec2(400,300));
            #ifdef DEBUG_BUILD
            if (!freqDomain) {

                ImPlot::BeginPlot("Signal", "Samples", "Amplitude");
                ImPlot::PlotLine("Signal", time_domain_labels.data(), time_domain_data.data(), time_domain_data.size());
                ImPlot::EndPlot();
                
            } else {
            #endif
                ImPlot::SetNextPlotLimitsX(0.0f, static_cast<double>(device.sampleRate/2));
                const double ticks[] = {0.0,0.05,0.2,0.5,1,4,8,20};
                ImPlot::SetNextPlotTicksX(ticks, 10);
                ImPlot::BeginPlot("FFT", "Frequency (kHz)", "Power (dBFS)", ImVec2(-1, 0), ImPlotFlags_None, ImPlotAxisFlags_LogScale);
                ImPlot::PlotLine("Power", this->freqs.data(), this->output.data()+1, (this->fft_size/2)-1);
                ImPlot::EndPlot();
            #ifdef DEBUG_BUILD
            }
            #endif
            ImGui::EndChildFrame();
        }

    ImNodes::EndNode();

    ImNodes::PopColorStyle();
    ImNodes::PopColorStyle();

}

void AnalyzerNode::ApplyFX(const float *in, float *out, size_t numFrames, AudioInfo info) {

    memcpy(out, in, numFrames * sizeof(float));
    size_t needed_size = fftconvolver::NextPowerOf2(std::max((size_t)4096,numFrames));
    fft_size = needed_size;
    if (needed_size != this->signal.size()) {
        internal_fft.init(needed_size);
        signal.resize(needed_size, 0.0f);
        real.resize(audiofft::AudioFFT::ComplexSize(needed_size), 0.0f);
        imaginary.resize(audiofft::AudioFFT::ComplexSize(needed_size), 0.0f);
        output.resize(needed_size);
        time_domain_data.resize(numFrames);
        time_domain_labels.resize(numFrames);
        std::fill(time_domain_data.begin(), time_domain_data.end(), 0.0f);
        for (size_t i = 0; i < numFrames; i++) {
            time_domain_labels[i] = i;
        }
    }

    for (size_t i = 0; i < signal.size(); i++) {
        // copy input buffer
        if (i < numFrames) {
            time_domain_data[i] = in[i];
            signal[i] = in[i];
            signal[i] *= dsp::blackman_harris_window(i, signal.size()-1);
        } else {
            signal[i] = 0.0f;
        }
    }


    this->internal_fft.fft(this->signal.data(), this->real.data(), this->imaginary.data());
    this->output_lock.lock();
    for (size_t i = 0; i < this->real.size(); i++) {
        output[i] = (this->real[i] * this->real[i]) + (this->imaginary[i] * this->imaginary[i]);
        output[i] = dsp::f32_to_dbfs(sqrt(output[i]));
    }
    this->output_lock.unlock();

    if (this->freqs.size() != this->output.size()) {
        this->freqs.resize(this->output.size());
        for (size_t i = 0; i < this->output.size(); i++) {
            freqs[i] = ((static_cast<float>(i) * static_cast<float>(device.sampleRate)) / static_cast<float>(needed_size))/1000.0f;
        }
    }

}