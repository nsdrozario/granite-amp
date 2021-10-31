
#pragma once

#include <mindsp/util/ring_buffer.hpp>
#include <MiddleNode.hpp>
#define CHORUS_VOICES 2


namespace guitar_amp {
    class ChorusNode : public MiddleNode {
    public:
        
        struct ChorusVoiceSettings {
            float avg_delay = 20; // ms
            float width = 5; // ms
            float depth = 50; // percent
            float frequency = 0.7;
        };

        ChorusNode(int id, const AudioInfo info);
        ChorusNode(int id, const AudioInfo current_audio_info, const sol::table &init_table);
        virtual ~ChorusNode();

        void showGui();
        void ApplyFX(const float *in, float *out, size_t numFrames, AudioInfo info);

        void luaInit(const sol::table &config);
        sol::table serializeLua();

    private:

        bool need_reinit = false;
        float internal_timer = 0;
        mindsp::util::ring_buffer<float> delay_lines[2];
        ChorusVoiceSettings delay_line_settings[2];

    };
}
