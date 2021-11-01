#include <ConfigManager.hpp>
#include <fstream>
#include <iostream>

namespace guitar_amp {

    void init_lua_for_config(sol::state &l) {
        l.open_libraries(sol::lib::base, sol::lib::string, sol::lib::table, sol::lib::package);    
    }

    std::string open_entire_file(std::string file_name) {
        std::ifstream file (file_name);
        return std::string(std::istreambuf_iterator<char>(file), std::istreambuf_iterator<char>());
    }

    CabSimSettings read_cabsim_config(std::string file_name) {
        sol::state l;
        std::string file_contents = open_entire_file(file_name);
        std::cout << file_contents << std::endl;
        CabSimSettings out;

        init_lua_for_config(l);
        l.do_file("scripts/io_module.lua");
        
        sol::function parse = l["read_config_file"];
        sol::table config = parse.call<sol::table>(file_contents);

            FilterDescriptor hpf;
            FilterDescriptor lpf;
            FilterDescriptor lowmid;
            FilterDescriptor mid;
            FilterDescriptor presence;
            std::cout << config.get<float>("Delay") << std::endl;
            out.lpf.freq = config.get_or("LowPassFreq", 4000.f);
            out.lpf.q = config.get_or("LowPassQ", 1.f);
            
            out.hpf.freq = config.get_or("HighPassFreq", 60.f);
            out.hpf.q = config.get_or("HighPassQ", 1.f);

            out.mid.freq = config.get_or("MidFreq",600.f);
            out.mid.gain_db = config.get_or("MidGain",0.f);
            out.mid.q = config.get_or("MidQ",4.f);

            out.lowmid.freq = config.get_or("LowMidFreq",250.f);
            out.lowmid.gain_db = config.get_or("LowMidGain",0.f);
            out.lowmid.q = config.get_or("LowMidQ",4.f);

            out.presence.freq = config.get_or("PresenceFreq",1200.f);
            out.presence.gain_db = config.get_or("PresenceGain",0.f);
            out.presence.q = config.get_or("PresenceQ",12.f);

            /*
            out.hpf = hpf;
            out.lpf = lpf;
            out.mid = mid;
            out.lowmid = lowmid;
            out.presence = presence;
            */
            out.delay_ms = config.get_or("Delay", 1.0f);
       
        
        return out;

    }
    void save_cabsim_config(CabSimSettings settings, std::string file_name) {
        std::ofstream file("assets/cabsim_presets/" + file_name);
        file << "LowPassFreq: "  << settings.lpf.freq << std::endl;
        file << "LowPassQ: " << settings.lpf.q<< std::endl;
        file << "HighPassFreq: " << settings.hpf.freq<< std::endl;
        file << "HighPassQ: " << settings.hpf.q << std::endl;
        file << "MidFreq: " << settings.mid.freq<< std::endl;
        file << "MidGain: "     << settings.mid.gain_db << std::endl;
        file << "MidQ: " << settings.mid.q<< std::endl;
        file << "LowMidFreq: " << settings.lowmid.freq<< std::endl;
        file << "LowMidGain: "    << settings.lowmid.gain_db<< std::endl; 
        file << "LowMidQ: " << settings.lowmid.q << std::endl;
        file << "PresenceFreq: " << settings.presence.freq << std::endl;
        file << "PresenceGain: " << settings.presence.gain_db << std::endl;
        file << "PresenceQ: " << settings.presence.q << std::endl;
        file << "Delay: " << settings.delay_ms << std::endl;
        file.close();
    }
}