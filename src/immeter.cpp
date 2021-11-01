#include <immeter.hpp>

void ImMeter::Meter(const char *label, float *val, float min, float max) {
    
    ImGuiStyle &style = ImGui::GetStyle();
    ImDrawList *draw_list = ImGui::GetWindowDrawList();
    ImGuiIO &io_handle = ImGui::GetIO();
    ImVec2 draw_pos = ImGui::GetCursorScreenPos();
    ImVec2 text_size = ImGui::CalcTextSize(label);
    draw_pos.x += 20;
    draw_pos.y += 30;

    float width = ImGui::GetWindowWidth() - 60;
    
    auto green_col = IM_COL32(3, 252, 73, 100);

    ImGui::PushID(val);
    ImGui::Dummy(ImVec2(width, 90));

    draw_list->AddText(ImVec2(draw_pos.x + (width/2) - (text_size.x / 2), draw_pos.y - 30), IM_COL32(255,255,255,255), label);
    draw_list->AddRectFilled(draw_pos, ImVec2(draw_pos.x + width, draw_pos.y + 20), IM_COL32(39, 116, 161, 255));
    if ((*val - min) / (max-min) >= 0.0f) {
        if ((*val - min) / (max-min) > 1.0f) {
            draw_list->AddRectFilled(draw_pos, ImVec2(draw_pos.x + width, draw_pos.y + 20), IM_COL32(0, 200, 255, 255));
        } else {
            draw_list->AddRectFilled(draw_pos, ImVec2(draw_pos.x + ( width * (*val - min) / (max-min)), draw_pos.y + 20), IM_COL32(0, 200, 255, 255), 5);
        }
    }
    for (int i = 0; i <= 4; i++) {
        char buf[256];
        snprintf(buf, 256, "%.1f", min + (i * (max-min)/4));
        ImVec2 text_size = ImGui::CalcTextSize(buf);
        draw_list->AddLine(ImVec2(draw_pos.x + (i * width / 4) + 1, draw_pos.y), ImVec2(draw_pos.x + (i * width / 4) + 1, draw_pos.y+25), IM_COL32(255,255,255,150), 2.0f);
        draw_list->AddText(ImVec2(draw_pos.x + (i * width / 4) - (text_size.x / 2), draw_pos.y+15+(text_size.y)), IM_COL32(255,255,255,255), buf);
    }

    ImGui::PopID();

}