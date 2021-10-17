#ifndef GUITAR_AMP_EQNODE_HPP
#define GUITAR_AMP_EQNODE_HPP

#include "MiddleNode.hpp"
#include "state.hpp"
#include <list>
#include <utility>
#include "internal_dsp.hpp"
#include "miniaudio.h"
#include <mindsp/filter.hpp>

namespace guitar_amp {

    enum class EQType {
        LowPass,
        HighPass,
        LowShelf,
        HighShelf,
        Peak,
        Notch
    };

    class FilterWrapper {
        EQType filterType;
        mindsp::filter::biquad_filter filter;
        float gain;
        float sample_rate;
        float q;
        float freq;
    };

    class EQNode : public MiddleNode {
    public:
        EQNode(int id, const AudioInfo current_audio_info);
        EQNode(int id, const AudioInfo current_audio_info, const sol::table &init_table);
        virtual ~EQNode();

        void showGui();
        void ApplyFX(const float *in, float *out, size_t numFrames, AudioInfo info);
        void luaInit(const sol::table &init_table);
        virtual sol::table serializeLua();

    private:
        std::list<FilterWrapper> filters;  
    };
}

#endif