#ifndef GUITAR_AMP_FILTER_WRAPPER_HPP
#define GUITAR_AMP_FILTER_WRAPPER_HPP

#include <miniaudio.h>

namespace guitar_amp {
    
    struct FilterDescriptor {
        
        enum class FilterType {
            LowPass,
            HighPass,
            Peak,
            Notch,
            LowShelf,
            HighShelf
        };
        
        FilterType type;
        double q;
        double gain_db;
        double freq; 
    
    };

    struct CabSimSettings {
        FilterDescriptor lpf;
        FilterDescriptor hpf;
        FilterDescriptor mid;
        FilterDescriptor lowmid;
        FilterDescriptor presence;
        double delay_ms;
    };

    struct OverdriveSettings {

    };

}

#endif