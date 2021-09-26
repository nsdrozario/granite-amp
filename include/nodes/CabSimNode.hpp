#ifndef GUITAR_AMP_CAB_SIM_NODE_HPP
#define GUITAR_AMP_CAB_SIM_NODE_HPP

#include "headers.hpp"
#include "internal_dsp.hpp"
#include "state.hpp"
#include "MiddleNode.hpp"
#include <mindsp/filter.hpp>
#include "../ConfigManager.hpp"
#include <sol/sol.hpp>

namespace guitar_amp {

    class CabSimNode : public MiddleNode {
    public:

        CabSimNode(int id, const AudioInfo current_audio_info);
        CabSimNode(int id, const AudioInfo current_audio_info, const sol::table &init_table);
        virtual ~CabSimNode();

        void reinit(CabSimSettings settings);

        void showGui();
        void ApplyFX(const float *in, float *out, size_t numFrames, AudioInfo info);
        void luaInit(sol::table init_lua);

    private:

        std::vector<std::string> preset_files;

        // for comb filtering
        dsp::ring_buffer<float> delay;

        /*
        // cabs typically output around 6khz at most
        ma_lpf2 lpf;

        // some cabs dont have very much bass at all
        ma_hpf2 hpf;

        // for making the cabinet sound thicker
        ma_peak2 low_mids_boost;

        // the annoying 1-1.5khz region
        ma_peak2 presence;

        ma_peak2 mid_scoop;
        */

        mindsp::filter::biquad_filter lpf;
        mindsp::filter::biquad_filter hpf;
        mindsp::filter::biquad_filter low_mid;
        mindsp::filter::biquad_filter mid;
        mindsp::filter::biquad_filter presence;

        // frequencies
        float lpf_freq = 4000.0f;
        float hpf_freq = 60.0f; // 80hz is about an E2 but when we pile up distortion it really doesn't matter to have the fundamental frequency
        float mid_scoop_freq = 500.0f;
        float low_mids_boost_freq = 250.0f;
        float presence_freq = 1250.0f;

        // magnitudes (in decibels)
        float presence_magnitude = -6.0f;
        float mid_scoop_magnitude = -12.0f;
        float low_mids_boost_magnitude = 6.0f;  

        // q factor
        float lpf_q = 2.0f;
        float hpf_q = 2.0;
        float mid_scoop_q = 4.0f;
        float presence_q = 12.0f;
        float low_mids_boost_q = 4.0f;

        float delay_time = 1.0f; // in milliseconds

        bool changed_presence = false;
        bool changed_lpf = false;
        bool changed_hpf = false;
        bool changed_low_mids_boost = false;
        bool changed_mid_scoop = false;
        bool changed_delay = false;

    };

}

#endif