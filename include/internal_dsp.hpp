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

        class blackman_harris_window {
        public:
            blackman_harris_window();
            ~blackman_harris_window();

            void apply_window(const float *in, float *out, size_t num_frames);

        private:
            size_t sample_size;
            size_t sample_rate;
        };

    }
}

#endif