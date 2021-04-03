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
#ifndef GUITAR_AMP_INTERNAL_DSP_HPP
#define GUITAR_AMP_INTERNAL_DSP_HPP

#include "headers.hpp"
#define _USE_MATH_DEFINES
#include <cmath>
#include <vector>

namespace guitar_amp {
    namespace dsp {

        // Clipping / saturation algorithms
        void hardclip_minmax(const float *input, float *transform, float gain, float threshold, ma_uint32 frameCount);
        void clip_tanh(const float *input, float *output, float gain, float output_volume, ma_uint32 frameCount);
        void clip_sin(const float *input, float *output, float gain, float output_volume, ma_uint32 frameCount);

        // Utility
        float f32_to_dbfs(float x);
        float dbfs_to_f32(float x);

        size_t seconds_to_samples(float time, size_t sample_rate);

        // classes
        class ring_buffer : public std::vector<float> {
            public:
                ring_buffer();
                virtual ~ring_buffer();
                ring_buffer(size_t block_size, size_t samples);
                virtual float& operator[](size_t input);
        };

    }
}

#endif