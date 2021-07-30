#ifndef IMKNOB_HPP
#define IMKNOB_HPP

#include <imgui.h>
#include <algorithm>
#include <cmath>
#include <string>
#include <sstream>
#include <cstdio>

namespace ImKnob {
    bool Knob(const char *label, float *value, float speed, float min, float max, const char *format="%.3f", float radius=32.0f, ImVec4 color=ImVec4(0.2f,0.2f,0.2f,1.0f), ImVec4 color_active=ImVec4(0.3f,0.3f,0.3f,1.0f));
}

#endif