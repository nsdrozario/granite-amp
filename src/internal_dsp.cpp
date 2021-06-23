#include <internal_dsp.hpp>
#include <algorithm>
#include <cmath>
#include "state.hpp"
using namespace guitar_amp;

float guitar_amp::dsp::f32_to_dbfs(float x) {
    return 20.0f * log10 ( abs(x) );
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

dsp::delay_line::delay_line() {
    if (device.sampleRate) {
        
    } else {

    }
}