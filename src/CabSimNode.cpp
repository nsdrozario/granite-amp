#include <CabSimNode.hpp>
using namespace guitar_amp;

std::mutex noise_lock;
std::vector<float> brownian_noise;

CabSimNode::CabSimNode(int id) : MiddleNode(id) {

    // set up noise impulse if it isn't set up already
    if (brownian_noise.size() == 0) {
        size_t noise_length = (device.sampleRate) ? device.sampleRate : 48000; // 1 second of brownian noise
        ma_noise_config cfg = ma_noise_config_init(ma_format_f32, 1, ma_noise_type_brownian, 100, dsp::dbfs_to_f32(-10.0f));
        ma_noise noise_gen;

        ma_noise_init(&cfg, &noise_gen);
        noise_lock.lock();
        brownian_noise.resize(noise_length);
        ma_noise_read_pcm_frames(&noise_gen, brownian_noise.data(), noise_length);
    }

    // set up convolver
    size_t convolver_block_size = (device.capture.internalPeriodSizeInFrames) ? device.capture.internalPeriodSizeInFrames : 256;
    convolver.init(convolver_block_size, brownian_noise.data(), brownian_noise.size());
    
}

void CabSimNode::ApplyFX(const float *in, float *out, size_t numFrames) {
    memcpy(out, in, numFrames * sizeof(float));
    // implement later
}