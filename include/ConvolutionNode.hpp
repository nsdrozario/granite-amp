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

#include "MiddleNode.hpp"
#include <kfr/all.hpp>
#include "internal_dsp.hpp"

namespace guitar_amp {
    class ConvolutionNode : public MiddleNode {
    
    public:
        
        ConvolutionNode(int id) : MiddleNode(id) { }
        void showGui();
        void ApplyFX(float *in, float *out, size_t numFrames); 

    protected: 
    
        AudioProcessorNode *input;
        AudioProcessorNode *output;
    
    private:

        kfr::univector<float> *impulse;
        
    };
}

#endif