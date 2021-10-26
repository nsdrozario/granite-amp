#pragma once

#include <imgui.h>
#include <algorithm>
#include <cmath>
#include <string>
#include <sstream>
#include <cstdio>

namespace ImMeter {
    // Intended as an audio meter
    // Please convert to dbfs first
    void Meter(const char *label, float *val, float min, float max);
}