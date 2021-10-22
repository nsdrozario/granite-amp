#include <imknob.hpp>

bool ImKnob::Knob(const char *label, float *value, float speed, float min, float max, const char *format, float radius, ImVec4 color, ImVec4 color_active) {
    
    float text_padding = 20.0f;
    float drag_deadzone = 0.3f;
    float big_radius = 1.25f * radius;
    
    ImGuiStyle &style = ImGui::GetStyle();
    ImDrawList *draw_list = ImGui::GetWindowDrawList();
    ImGuiIO &io_handle = ImGui::GetIO();
    ImVec2 draw_position = ImGui::GetCursorScreenPos();
    ImVec2 label_size = ImGui::CalcTextSize(label);

    ImVec2 text_position (draw_position.x + text_padding, draw_position.y);
    ImVec2 center_position = ImVec2(draw_position.x+std::max((label_size.x*0.5f)+text_padding,radius), draw_position.y+big_radius+text_padding);
    
    // there is no reason for the number to be that long but it's just for safety
    char formatted[256];
    snprintf(formatted, 256, format, *value);
    ImVec2 value_text_position = text_position;
    value_text_position.y = draw_position.y + (2*big_radius) + text_padding + 5;
    ImVec2 value_text_size = ImGui::CalcTextSize(formatted);
    
    /*
    if (value_text_size.x < label_size.x) {
        value_text_position.x += (label_size.x/2) - (value_text_size.x/2);
    }
    */
    // consider this for the bottom one as well

    float bigger_label_size_x = std::max(label_size.x, value_text_size.x);
    if (bigger_label_size_x > big_radius) {
        // move the knob towards the center
        center_position.x = draw_position.x + text_padding + (bigger_label_size_x / 2);
        center_position.y = draw_position.y + big_radius + text_padding;
    } else {
        // move the text towards the center
        text_position.x = draw_position.x + text_padding + big_radius - (label_size.x/2);
        text_position.y = draw_position.y;
    }

    
    value_text_position.x = center_position.x - (value_text_size.x/2);
    text_position.x = center_position.x - (label_size.x/2);

    bool interacted = false;
    ImVec4 color_to_use = color_active;
    ImVec4 shadow_color = color;

    /* 
        the "min angle" and "max angle" are actually opposites; positive direction for angles is counterclockwise
        but positive direction for knob adjustments is clockwise
    */ 
    float min_angle = 1.25f * 3.141593f; // 5pi/4
    float max_angle = 3.141593f * -0.25f; // -pi/4

    *value = ImKnob::clamp(*value, min, max);
    float norm_position = (*value-min)/(max-min);
    float angle = (1.0f-norm_position) * (min_angle - max_angle) + max_angle;

    ImGui::PushID(value); // the variable being pointed to shouldn't be using other imgui elements
    ImGui::InvisibleButton("Knob", ImVec2(std::max(radius*2, bigger_label_size_x)+((text_padding*2)), (big_radius*2)+(label_size.y)+(value_text_size.y)+text_padding));

    // click action
    if (ImGui::IsItemClicked(ImGuiMouseButton_Left)) {
        ImVec2 click_position = io_handle.MouseClickedPos[0]; // 0 is left click
        ImVec2 delta_from_center = ImVec2(click_position.x - center_position.x, -(click_position.y - center_position.y));
        /*
        if (delta_from_center.x > 0.0f) {
            angle = std::atan(delta_from_center.y / delta_from_center.x);
        } else {
            angle = std::atan(delta_from_center.y / delta_from_center.x) + (3.141593f);
        }
        */
        angle = std::atan2(delta_from_center.y, delta_from_center.x);
        // normalize the angle then scale to the actual values
        *value = ImKnob::clamp((1.0f-((angle - max_angle) / (min_angle - max_angle))) * (max-min) + min, min, max);
        interacted = true;
    }

    // drag action
    if (ImGui::IsItemActive()) {
        // this way the mouse can move in an arc and the knob will follow correctly
        ImVec2 mouse_pos = io_handle.MousePos;
        ImVec2 delta_from_center = ImVec2(mouse_pos.x - center_position.x, -(mouse_pos.y - center_position.y));
        if (delta_from_center.x > drag_deadzone) {
            angle = std::atan(delta_from_center.y / delta_from_center.x);
        } else if (delta_from_center.x < -drag_deadzone) {
            angle = std::atan(delta_from_center.y / delta_from_center.x) + (3.141593f);
        } else {
            // this prevents an angle 0 issue where the knob immediately goes to one of the maximum positions
            angle = 3.141593 * 0.5;
        }
        angle = ImKnob::clamp(angle, max_angle, min_angle);
        // normalize the angle then scale to the actual values
        *value = ImKnob::clamp((1.0f-((angle - max_angle) / (min_angle - max_angle))) * (max-min) + min, min, max);
        interacted = true;
    }

    if (interacted || ImGui::IsItemHovered()) {
        color_to_use = color_active;
        color_to_use.x *= 1.1;
        color_to_use.y *= 1.35;
        color_to_use.z *= 1.7;
        shadow_color = color_active;
        shadow_color.y *= 1.1;
        shadow_color.z *= 1.3;
    }
    
    ImGui::PopID();
    
    // line should be about half of the radius in length
    // the positive y direction is down so to imitate a polar coordinate system the y component must be multiplied by -1
    ImVec2 point1 = ImVec2((radius * std::cos(angle)) + center_position.x, -(radius * std::sin(angle)) + center_position.y);
    ImVec2 point2 = ImVec2((radius * 0.5f * std::cos(angle)) + center_position.x, -(radius * 0.5f * std::sin(angle)) + center_position.y);

    draw_list->AddText(text_position, IM_COL32(255,255,255,255) ,label);
    draw_list->AddCircleFilled(ImVec2(center_position.x, center_position.y + (big_radius-radius)), big_radius, ImColor(shadow_color));
    draw_list->AddCircleFilled(center_position, radius, ImColor(color_to_use));
    draw_list->AddLine(point2, point1, IM_COL32(255,255,255,255), 2.0f);
    
 

    draw_list->AddText(value_text_position,IM_COL32(255,255,255,255), formatted);
    return interacted;
}