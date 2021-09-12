#include <ConfigManager.hpp>

namespace guitar_amp {

    void init_lua_for_config(sol::state &l) {
        l.open_libraries(sol::lib::base, sol::lib::string, sol::lib::table, sol::lib::package);    
    }

    std::string open_entire_file(std::string file_name) {
        std::string file_contents;
        std::ifstream file (file_name);
        if (file.good() && file.is_open()) {
            file.seekg(0, std::ios::end);
            std::size_t file_size = file.tellg();
            file.seekg(0, std::ios::beg);
            file_contents.resize(file_size);
            file.read(file_contents.data(), file_size);
            file.close();
            return file_contents;
        } else {
            file.close();
            return std::string("");
        }
    }

    CabSimSettings read_cabsim_config(std::string file_name) {
        sol::state l;
        std::string file_contents = open_entire_file(file_name);
        CabSimSettings out;

        init_lua_for_config(l);
        l.script_file("scripts/io_module.lua");
        
        sol::function parse = l["read_config_file"];
        sol::table config = parse.call<sol::table>(file_contents);
        if (!config.empty()) {
            out.delay_ms = 1.0f;
           
            FilterDescriptor hpf;
            FilterDescriptor lpf;
            FilterDescriptor lowmid;
            FilterDescriptor mid;
            FilterDescriptor presence;

            lpf.freq = config["LowPassFreq"];
            lpf.q = config["LowPassQ"];
            
            hpf.freq = config["HighPassFreq"];
            hpf.q = config["HighPassQ"];

            mid.freq = config["MidFreq"];
            mid.gain_db = config["MidGain"];
            mid.q = config["MidQ"];

            lowmid.freq = config["LowMidFreq"];
            lowmid.gain_db = config["LowMidGain"];
            lowmid.q = config["LowMidQ"];

            presence.freq = config["PresenceFreq"];
            presence.gain_db = config["PresenceGain"];
            presence.q = config["PresenceQ"];

            out.hpf = hpf;
            out.lpf = lpf;
            out.mid = mid;
            out.lowmid = lowmid;
            out.presence = presence;
            out.delay_ms = 1.0f;

        }
        
        return out;

    }

}