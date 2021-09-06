#ifndef GUITAR_AMP_IO_UTIL_HPP
#define GUITAR_AMP_IO_UTIL_HPP

#include "state.hpp"
#include "headers.hpp"
#include <iostream>

namespace guitar_amp {
    namespace io {
        bool refresh_devices();
        std::vector<float> read_entire_file_wav(std::string file_name, size_t sample_rate);
    }

    

}

#endif