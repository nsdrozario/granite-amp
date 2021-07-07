#include "miniaudio.h"
#ifndef GUITAR_AMP_AUDIOINFO
#define GUITAR_AMP_AUDIOINFO

namespace guitar_amp {
    struct AudioInfo {

        AudioInfo() {
            sample_rate = 0;
            period_length = 0;
            channels = 0;
        }

        AudioInfo(const AudioInfo &a) {
            sample_rate=a.sample_rate;
            period_length=a.period_length;
            channels=a.channels;
        }

        ma_uint32 sample_rate;
        size_t period_length;
        size_t channels; // really don't need to worry about this yet the app is in mono for now
    };
}

#endif