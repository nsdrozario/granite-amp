#pragma once

#include <sol/sol.hpp>
#include <fstream>
#include "ConfigWrappers.hpp"

namespace guitar_amp {

    void init_lua_for_config(sol::state &l);
   
    std::string open_entire_file(std::string file_name);

    CabSimSettings read_cabsim_config(std::string file_name);

}