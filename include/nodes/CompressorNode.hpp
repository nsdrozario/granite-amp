#ifndef GUITAR_AMP_COMPRESSORNODE_HPP
#define GUITAR_AMP_COMPRESSORNODE_HPP
#include "MiddleNode.hpp"
#include "internal_dsp.hpp"

namespace guitar_amp {
    class CompressorNode : public MiddleNode {
    public:
        CompressorNode(int id, const AudioInfo current_audio_info);
        CompressorNode(int id, const AudioInfo current_audio_info, const sol::table &init_table);

        virtual ~CompressorNode();    

        void showGui();
        void ApplyFX(const float *in, float *out, size_t numFrames, AudioInfo info);
        void luaInit(const sol::table &init_table);
        virtual sol::table serializeLua();
    
    private:
        static const int COMPRESSOR_PEAK = 0;
        // might add more metering types later
        
        // times are in seconds
        float attack = 0.0f;
        float release = 0.0f;
        float ratio = 2.0f;
        float threshold = -6.0f; // dB
        int metering = CompressorNode::COMPRESSOR_PEAK;
        
        size_t samples_remaining = 0;
        size_t samples_until_compress = 0;

        bool compressing = false;

        bool sidechain_enabled = false;

    };
}

#endif