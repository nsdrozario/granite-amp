/*
#pragma once

#include <mindsp/util/ring_buffer.hpp>
#include <MiddleNode.hpp>
#include <random>

namespace guitar_amp {
    class ChorusNode : public MiddleNode {
    public:
        
        ChorusNode(int id, const AudioInfo info);
        ChorusNode(int id, const AudioInfo current_audio_info, const sol::table &init_table);
        virtual ~ChorusNode();

        void showGui();
        void ApplyFX(const float *in, float *out, size_t numFrames, AudioInfo info);
        void luaInit(const sol::table &init_table);

    private:
        mindsp::util::ring_buffer<float> delay_line;
        float min_delay = 15.0f; // in ms
        float max_delay = 20.0f; // in ms

        size_t min_samples_delay = 0;
        size_t max_samples_delay = 0;
        std::random_device rand_device;
        std::default_random_engine rng;
        std::uniform_real_distribution<double> rand_dist;

    };
}
*/