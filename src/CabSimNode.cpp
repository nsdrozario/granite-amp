#include <CabSimNode.hpp>
using namespace guitar_amp;

std::mutex noise_lock;
std::vector<float> brownian_noise;

CabSimNode::CabSimNode(int id, const AudioInfo current_audio_info) : MiddleNode(id, current_audio_info) {

    // set up noise impulse if it isn't set up already
    if (brownian_noise.size() == 0) {
        size_t noise_length = (device.sampleRate) ? device.sampleRate : 48000; // 1 second of brownian noise
        ma_noise_config cfg = ma_noise_config_init(ma_format_f32, 1, ma_noise_type_brownian, 100, dsp::dbfs_to_f32(-10.0f));
        ma_noise noise_gen;

        ma_noise_init(&cfg, &noise_gen);
        noise_lock.lock();
        brownian_noise.resize(noise_length);
        ma_noise_read_pcm_frames(&noise_gen, brownian_noise.data(), noise_length);
        noise_lock.unlock();
    }

    // set up convolver
    size_t convolver_block_size = (device.capture.internalPeriodSizeInFrames) ? device.capture.internalPeriodSizeInFrames/2 : 256;
    convolver.init(convolver_block_size, brownian_noise.data(), brownian_noise.size());
    last_period_size = 512;
}

CabSimNode::~CabSimNode() { }

void CabSimNode::showGui() {
    ImGui::PushItemWidth(100);
    imnodes::PushColorStyle(imnodes::ColorStyle_TitleBar, IM_COL32(170,110,220, 100));
    imnodes::PushColorStyle(imnodes::ColorStyle_TitleBarSelected, IM_COL32(170,110,220, 255));
    imnodes::PushColorStyle(imnodes::ColorStyle_TitleBarHovered, IM_COL32(170,110,220, 255));
    
    imnodes::BeginNode(id);
        
        imnodes::BeginNodeTitleBar();
            ImGui::TextUnformatted("Algorithmic Cabinet Simulation");
        imnodes::EndNodeTitleBar();

        imnodes::PushAttributeFlag(imnodes::AttributeFlags::AttributeFlags_EnableLinkDetachWithDragClick);
        imnodes::BeginInputAttribute(this->id+1);
        imnodes::EndInputAttribute();

        imnodes::BeginOutputAttribute(this->id+3);
        imnodes::EndOutputAttribute();
        imnodes::PopAttributeFlag();
    imnodes::EndNode();

    ImGui::PopItemWidth();
    imnodes::PopColorStyle();
    imnodes::PopColorStyle();
}

void CabSimNode::ApplyFX(const float *in, float *out, size_t numFrames, AudioInfo info) {

    if (device.sampleRate != 0 && device.sampleRate != brownian_noise.size()) {
        noise_lock.lock();
        brownian_noise.resize(device.sampleRate);
        ma_noise_config cfg = ma_noise_config_init(ma_format_f32, 1, ma_noise_type_brownian, 100, dsp::dbfs_to_f32(-10.0f));
        ma_noise noise_gen;
        ma_noise_init(&cfg, &noise_gen);
        ma_noise_read_pcm_frames(&noise_gen, brownian_noise.data(), brownian_noise.size());
        noise_lock.unlock();
    }
    
    if (numFrames != last_period_size) {
        last_period_size = numFrames;
        convolver.reset();
        convolver.init(last_period_size/2, brownian_noise.data(), brownian_noise.size());
    }

    convolver.process(in, out, numFrames);

}