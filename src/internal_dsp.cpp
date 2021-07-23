#include <internal_dsp.hpp>
#include <algorithm>
#include <cmath>
#include "state.hpp"
#include <map>
#include <numeric>
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
size_t guitar_amp::dsp::seconds_to_samples(float time, size_t sample_rate) {
    return static_cast<size_t>(static_cast<float>(sample_rate) * time);
}

float guitar_amp::dsp::blackman_harris_window(size_t n, size_t N) {
    // access by mem[N][n]
    static std::unordered_map<size_t, std::vector<float>> mem;

    // these are the values for a 4-term blackman-harris window
    // https://web.mit.edu/xiphmont/Public/windows.pdf
    // page 15
    // or 
    // https://en.wikipedia.org/wiki/Window_function#Blackman%E2%80%93Harris_window
    // the values are the same in both sources
    float a_0 = 0.35875;
    float a_1 = 0.48829;
    float a_2 = 0.14128;
    float a_3 = 0.01168;

    // if the window for this specific length hasn't been calculated yet, initialize it as an array of -1
    if (mem.find(N) == mem.end()) {
        // this is safe as a default value because 
        // blackman-harris window shouldn't have negative values
        mem[N] = std::vector<float>(N, -1.0f); 
    }

    // if the window values have not already been calculated
    if (mem[N][n] == -1.0f) {
        // calculate them
        mem[N][n] = a_0 - (a_1 * cos(2 * M_PI * n / N)) + (a_2 * cos(4 * M_PI * n / N)) - (a_3 * cos(6 * M_PI * n / N));
        return mem[N][n];
    } else {
        // return the precalculated value otherwise
        return mem[N][n];
    }
}