/*
Copyright (c) 2021 Nathaniel D'Rozario

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
*/
#pragma once

#include <vector>
#include <cstddef>
#include <cmath>

namespace mindsp {
    namespace util {

        /**
         * 
         * Helper function to wrap array incides betweem 0 and the array size
         * 
         * @param val Value to wrap 
         * @param max Maximum value
         * 
         */
        std::size_t wrap_index(std::size_t val, std::size_t max) {
            return (val + max) % max;
        }
        
        template <typename T>
        class ring_buffer {
        public:
            
            ring_buffer() { }
            ring_buffer(std::size_t size) {
                buffer.resize(size);
                std::fill(buffer.begin(), buffer.end(), T());
            }
            virtual ~ring_buffer() { }
            
            void resize(std::size_t size) {
                buffer.resize(size);
            }

            /**
             * 
             * Reads a delayed value relative to the write pointer as if a tap were placed tap_index samples behind the write pointer. 
             * @param tap_index Number of samples behind the write pointer to read from
             * 
             */
            T read_tap(std::size_t tap_index) {
                std::size_t tap_index2 = tap_index % buffer.size();
                std::size_t index = wrap_index(write_ptr - tap_index2, buffer.size());
                return buffer[index];
            }
            
            /**
             * 
             * Fractional delay read using linear interpolation
             * @param tap_index Number of samples behind the write pointer to read from
             * 
             */
            T read_tap_lerp(float tap_index) {
                float size = static_cast<float>(buffer.size());
                float tap_index2 = std::fmod(tap_index, size);
                float index = std::fmod(static_cast<float>(write_ptr) - tap_index2, size);
                std::size_t x0 = static_cast<std::size_t> (index);
                float out = (buffer[(x0 + 1) % buffer.size()] - buffer[x0]) * (index - std::floor(index)) + buffer[x0];
                return out;
            }

            /**
             * 
             * Subscript operator (uses absolute indices instead of indices relative to the write pointer)
             * 
             * 
             */
            T& operator[](std::size_t index) {
                return buffer[index];
            }

            /**
             * 
             *  Places a value at the write pointer and increments the write pointer, wrapping it around the buffer size. 
             *  @param val Value to write at the write pointer's position
             * 
             */
            void push(T val) {
                buffer[write_ptr] = val;
                write_ptr = (write_ptr + 1) % buffer.size();
            }
        private:
            std::vector<T> buffer;
            std::size_t write_ptr = 0;
        };
    }

}