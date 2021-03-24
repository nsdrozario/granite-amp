/*
    Real time guitar amplifier simulation
    Copyright (C) 2021  Nathaniel D'Rozario

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License along
    with this program; if not, write to the Free Software Foundation, Inc.,
    51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
*/
#include <internal_dsp.hpp>
#include <algorithm>
#include <cmath>

float guitar_amp::dsp::f32_to_dbfs(float x) {
    return 20.0f * log10 ( fabsf(x) );
}

float guitar_amp::dsp::dbfs_to_f32(float x) {
    return powf(10, x / 20.0f);
}

void guitar_amp::dsp::hardclip_minmax(const float *input, float *transform, float gain, float threshold, ma_uint32 frameCount) {
    float real_gain = dbfs_to_f32(gain);
    float real_threshold = dbfs_to_f32(threshold);
    for (ma_uint32 i = 0; i < frameCount; i++) {
        if (input[i] > 0) {
            transform[i] = std::min(input[i]*real_gain, real_threshold);
        } else {
            transform[i] = std::max(input[i]*real_gain, -real_threshold);
        }
    }
}

void guitar_amp::dsp::clip_tanh(const float *input, float *output, float gain, float output_level, ma_uint32 frameCount) {
    float real_gain = dbfs_to_f32(gain);
    float real_output_level = dbfs_to_f32(output_level);
    for (ma_uint32 i = 0; i < frameCount; i++) {
        output[i] = real_output_level * tanhf(input[i] * real_gain / real_output_level);
    }
}

void guitar_amp::dsp::clip_sin(const float *input, float *output, float gain, float output_level, ma_uint32 frameCount) {
    float real_gain = dbfs_to_f32(gain);
    float real_output_level = dbfs_to_f32(output_level);
    float comp = 1.57079632679f * real_output_level; // x position of abs max of a*sin(a/x) one half cycle to the left and right of the origin
    for (ma_uint32 i = 0; i < frameCount; i++) {
        if (input[i] * real_gain > comp) { 
            output[i] = real_output_level;
        } else if (input[i] * real_gain < -comp) { //-pi/2
            output[i] = -real_output_level;
        } else {
            output[i] = real_output_level * sinf(input[i] * real_gain / real_output_level);
        }
    }
}
