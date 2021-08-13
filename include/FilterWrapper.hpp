#ifndef GUITAR_AMP_FILTER_WRAPPER_HPP
#define GUITAR_AMP_FILTER_WRAPPER_HPP

#include <miniaudio.h>

namespace guitar_amp {
    
    enum class FilterType {
        LowPass,
        HighPass,
        Peak,
        Notch,
        HighShelf,
        LowShelf,
        OtherBiquad
    };

    class FilterWrapper {
    public:
        FilterWrapper();
        FilterType type = FilterType::OtherBiquad;
    protected:
    };

}

#endif