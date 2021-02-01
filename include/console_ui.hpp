#ifndef GUITAR_AMP_CONSOLE_UI_HPP
#define GUITAR_AMP_CONSOLE_UI_HPP

#include <cstdio>
#include <cstring>
extern "C" {
    #include "miniaudio.h"
}
#include "state.hpp"

namespace guitar_amp {
    void list_devices(ma_device_info *deviceList, ma_uint32 n);
    void consoleInputPrompt();
}

#endif