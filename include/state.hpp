#ifndef GUITAR_AMP_STATE_HPP
#define GUITAR_AMP_STATE_HPP

extern "C" {
    #include "miniaudio.h"
}

#include <kfr/all.hpp>
#include <SFML/Graphics.hpp>

#define NK_INCLUDE_FIXED_TYPES
#define NK_INCLUDE_STANDARD_IO
#define NK_INCLUDE_STANDARD_VARARGS
#define NK_INCLUDE_DEFAULT_ALLOCATOR
#define NK_INCLUDE_VERTEX_BUFFER_OUTPUT
#define NK_INCLUDE_FONT_BAKING
#define NK_INCLUDE_DEFAULT_FONT
#include "nuklear.h"
#include "nuklear_sfml.hpp"

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
        static kfr::univector2d<float> ir;

        static sf::RenderWindow renderWindow;

    };
}

#endif