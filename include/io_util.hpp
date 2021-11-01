#ifndef GUITAR_AMP_IO_UTIL_HPP
#define GUITAR_AMP_IO_UTIL_HPP

#include "state.hpp"
#include "headers.hpp"
#include <iostream>
#include <filesystem>

namespace guitar_amp {
    namespace io {
        
        bool refresh_devices();
        std::vector<float> read_entire_file_wav(std::string file_name, size_t sample_rate);

        void file_paths(std::vector<std::string> &path_vector, std::string directory_path);
        void file_names(std::vector<std::string> &name_vector, std::string directory_path);
        void align_c_str_vector(std::vector<std::string> &in, std::vector<const char *> &out);
    }

    

}

#endif