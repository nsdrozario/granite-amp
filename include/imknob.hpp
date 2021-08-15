#ifndef IMKNOB_HPP
#define IMKNOB_HPP

#include <imgui.h>
#include <algorithm>
#include <cmath>
#include <string>
#include <sstream>
#include <cstdio>

namespace ImKnob {
    template <class T> 
    T clamp (T val, T min, T max) {
        return std::min(std::max(val, min), max);
    }
    bool Knob(const char *label, float *value, float speed, float min, float max, const char *format="%.3f", float radius=24.0f, ImVec4 color=ImVec4(0.2f,0.2f,0.2f,1.0f), ImVec4 color_active=ImVec4(0.3f,0.3f,0.3f,1.0f));
}

#endif