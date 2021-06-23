#ifndef GUITAR_AMP_INTERNAL_DSP_HPP
#define GUITAR_AMP_INTERNAL_DSP_HPP

#include "headers.hpp"
#define _USE_MATH_DEFINES
#include <cmath>
#include <vector>

#define MAX_DELAY_DURATION 10.0

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
        template <class T>
        class ring_buffer {
        public:
            ring_buffer();
            ring_buffer(size_t size);
            
            size_t inc_read_ptr() {
                
            }
            size_t inc_write_ptr() {

            }

            size_t get_read_ptr_index() {

            }
            size_t get_write_ptr_index() {

            }

            T get_read_ptr_value() {

            }
            T get_write_ptr_value() {

            }
            
            void set_write_ptr_value(size_t index, T val) {

            };
        private:
            size_t read_ptr;
            size_t write_ptr;
            size_t buf_size;
            T *buf;
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