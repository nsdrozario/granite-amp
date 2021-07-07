#include <EQNode.hpp>
#include <utility>
#include <algorithm>
#include <implot.h>

using namespace guitar_amp;

EQNode::EQNode(int id) : MiddleNode(id) {

}

EQNode::~EQNode() { }

void EQNode::showGui() {
    ImGui::PushItemWidth(100);
    imnodes::PushColorStyle(imnodes::ColorStyle_TitleBar, IM_COL32(0, 166, 255, 100));
    imnodes::PushColorStyle(imnodes::ColorStyle_TitleBarSelected, IM_COL32(0, 166, 255, 255));
    imnodes::PushColorStyle(imnodes::ColorStyle_TitleBarHovered, IM_COL32(0, 166, 255, 255));
    
    imnodes::BeginNode(id);
        
        imnodes::BeginNodeTitleBar();
            ImGui::TextUnformatted("EQ");
        imnodes::EndNodeTitleBar();

            ImGui::BeginChildFrame(this->id, ImVec2(400,300));
            ImPlot::SetNextPlotLimitsX(0.0f, static_cast<double>(device.sampleRate/2));
            const double ticks[] = {0.0,50.0,100.0,300.0,500.0,1000.0,2000.0,4000.0,10000.0,20000.0};
            ImPlot::SetNextPlotTicksX(ticks, 10);
            ImPlot::BeginPlot("EQ", "Frequency", "Power", ImVec2(-1, 0), ImPlotFlags_None, ImPlotAxisFlags_LogScale);

            for (size_t i = 0; i < this->points.size(); i++) {
                ImPlot::DragPoint(std::to_string(i).c_str(), &(this->points[i].first), &(this->points[i].second));
            }

            ImPlot::EndPlot();
            ImGui::EndChildFrame();

            if (ImGui::Button("Add Band")) {
                ma_peak2_config cfg = ma_peak2_config_init(ma_format_f32, 1, device.sampleRate, 0.0, 1.0, 1000.0);
                ma_peak2 tmp;
                if (ma_peak2_init(&cfg, &tmp) == MA_SUCCESS) {
                    this->filters.push_back(tmp);
                }
                this->points.push_back(std::make_pair(1000.0, 0.0));
            }

            if (ImGui::Button("Remove Last Band")) {
                if (this->filters.size() > 0 && this->points.size() > 0) {
                    this->filters.pop_back();
                    this->points.pop_back();
                }
            }

            ImGui::BeginChildFrame(this->id+1, ImVec2(400,200));
            size_t it_count = 0;
            for (auto it = this->filters.begin(); it != this->filters.end(); it++, it_count++) {
                ImGui::Text("Dummy text");
            }
            ImGui::EndChildFrame();
    imnodes::EndNode();

    ImGui::PopItemWidth();
    imnodes::PopColorStyle();
    imnodes::PopColorStyle();
}

void EQNode::ApplyFX(const float *in, float *out, size_t numFrames, const AudioInfo &info) {
    memcpy(out, in, numFrames * sizeof(float));
}