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
#ifndef GUITAR_AMP_STATE_HPP
#define GUITAR_AMP_STATE_HPP

#define MA_NO_DECODING
#define MA_NO_ENCODING

extern "C" {
    #include "miniaudio.h"
}

#include <kfr/all.hpp>
#include <SFML/Graphics.hpp>

namespace guitar_amp {
    class state {
    public:
        static ma_context              c;
        static ma_device_info *        outputDeviceInfo;
        static ma_device_info *        inputDeviceInfo; 
        static ma_uint32               numOutputDevices;
        static ma_uint32               numInputDevices;
        static ma_uint32               selectedInput;
        static ma_uint32               selectedOutput;
        static ma_device_config        deviceConf;
        static ma_device               device;
        static sf::RenderWindow renderWindow;
    };
}

#endif