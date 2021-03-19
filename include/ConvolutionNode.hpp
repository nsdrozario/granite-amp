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
#ifndef GUITAR_AMP_CONVOLUTIONNODE_HPP
#define GUITAR_AMP_CONVOLUTIONNODE_HPP
#define FFTCONVOLVER_DONT_USE_SSE

#include "MiddleNode.hpp"
#include "internal_dsp.hpp"
#include <string>
#include <thread>
#include <mutex>
#include "ImGuiFileBrowser.h"
#include <vector>
#include "FFTConvolver.h"

namespace guitar_amp {
    class ConvolutionNode : public MiddleNode {
    
    public:
        
        ConvolutionNode(int id);
        ~ConvolutionNode();
        
        void showGui();
        void ApplyFX(const float *in, float *out, size_t numFrames); 

    private:

        std::mutex impulseLock;
        std::vector<float> impulse;
        ma_decoder file_reader;
        fftconvolver::FFTConvolver convolver;
        bool bypass = true;
        imgui_addons::ImGuiFileBrowser file_browser;
        float gain = -144.0f;

        void loadIRFile(const std::string &path);

    };
}

#endif