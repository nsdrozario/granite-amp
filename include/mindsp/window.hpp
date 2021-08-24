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

#include <cmath>

namespace mindsp {
    namespace window {
        
        /**
         * 
         * Hann window 
         * @param n window index
         * @param N window size
         * 
         */
        float hann(std::size_t n, std::size_t N) {
            float sqrt_hann = std::sin(3.141592654f * n / (N-1));
            return sqrt_hann * sqrt_hann;
        }

        /**
         * 
         * Bartlett window (triangular window with L=N)
         * @param n window index
         * @param N window size
         *
         */
        float bartlett(std::size_t n, std::size_t N) {
            return 1.0f - std::abs( (n-(N*0.5)) / (N * 0.5) );
        }

        /**
         * 
         * Generalized cosine-sum window
         * @param n window index
         * @param N window size
         * @param a Array of coefficients
         * @param k Size of coefficient array
         * 
         */

        float cosine_sum(std::size_t n, std::size_t N, float *a, std::size_t k) {
            float out = 0.0f;
            for (std::size_t i = 0; i < k; i++) {
                float term = a[i] * std::cos(2.0f * i * 3.1415927f * n / N);
                if (i % 2 == 1) {
                    out -= term;
                } else {
                    out += term;
                }
            }
            return out;
        };

        /**
         * 
         * Blackman-Harris window
         * @param n window index
         * @param N window size
         * 
         */
        float blackman_harris(std::size_t n, std::size_t N) {
            float a[4] = {0.35875, 0.48829, 0.14128, 0.01168};
            return cosine_sum(n, N, a, 4);
        }

        /**
         * 
         * Hamming window
         * @param n window index
         * @param N window size
         * 
         */
        float hamming(std::size_t n, std::size_t N) {
            float a[2] = {0.54, 0.46};
            return cosine_sum(n, N, a, 2);
        }

    }
}