#ifndef GUITAR_AMP_INTERNAL_DSP_HPP
#define GUITAR_AMP_INTERNAL_DSP_HPP

#include "headers.hpp"
#define _USE_MATH_DEFINES
#include <cmath>
#include <vector>
#include <iostream>
#include <cstring>

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

        // Templated ring buffer class, uses new for memory allocation
        template <class T>
        class ring_buffer {
        public:
            // Default size 0 (unallocated memory). Read and write ptrs are at position 0. 
            ring_buffer() {
                buf_size = 0;
            }
            
            // Initialize ring buffer of length size, with read and write ptrs at position 0.
            ring_buffer(size_t size) {
                buf = new T[size];
                buf_size = size;
            }

            // Initialize ring buffer of length size, with read_ptr = init_read_ptr and write_ptr = init_write_ptr. Sets all values to 0.
            ring_buffer(size_t size, size_t init_read_ptr, size_t init_write_ptr) {
                buf = new T[size];
                buf_size = size;
                read_ptr = init_read_ptr;
                write_ptr = init_write_ptr;
                memset(buf, 0, size * sizeof(T));
            }

            void reinit(size_t size, size_t init_read_ptr, size_t init_write_ptr) {
                if (buf_size != 0) {
                    delete[] buf;
                }
                buf = new T[size];
                buf_size = size;
                read_ptr = init_read_ptr;
                write_ptr = init_write_ptr;
                memset(buf, 0, size * sizeof(T));
            }

            ~ring_buffer() {
                delete[] buf;
            }

            // Returns value of read_ptr
            size_t inc_read_ptr() {
                read_ptr = (read_ptr+1) % buf_size;
                return read_ptr;
            }

            // Returns value of write_ptr
            size_t inc_write_ptr() {
                read_ptr = (read_ptr+1) % buf_size;
                return read_ptr;
            }

            // Returns value of read_ptr
            size_t get_read_ptr_index() {
                return read_ptr;
            }

            // Returns value of write_ptr
            size_t get_write_ptr_index() {
                return write_ptr;
            }

            // Returns buf[read_ptr]
            T get_read_ptr_value() {
                return buf[read_ptr];
            }

            // Returns buf[write_ptr]
            T get_write_ptr_value() {
                return buf[write_ptr];
            }
            
            // Sets val at buf[write_ptr]
            void set_write_ptr_value(T val) {
                buf[write_ptr] = val;
            }

            size_t size() {
                return buf_size;
            }

        private:
            size_t read_ptr = 0;
            size_t write_ptr = 0;
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