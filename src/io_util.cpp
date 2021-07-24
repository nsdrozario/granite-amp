#include <io_util.hpp>

bool guitar_amp::io::refresh_devices() {

    ma_result result_get_devices = ma_context_get_devices
    (
        &context,
        &outputDevices,
        &numOutputDevices,
        &inputDevices,
        &numInputDevices
    );

    if (result_get_devices != MA_SUCCESS) {
        return false;
    }

    inputNames.resize(numInputDevices);
    outputNames.resize(numOutputDevices);

    for (ma_uint32 i = 0; i < numInputDevices; i++) {
        inputNames[i] = inputDevices[i].name;
    }

    for (ma_uint32 i = 0; i < numOutputDevices; i++) {
        outputNames[i] = outputDevices[i].name;
    }

    return true;
}

std::vector<float> guitar_amp::io::read_entire_file_wav(std::string file_name, size_t sample_rate) {

    ma_decoder_config decoder_cfg = ma_decoder_config_init(ma_format_f32, 1, sample_rate);
    ma_decoder decoder;
    std::vector<float> data;
    
    if (ma_decoder_init_file_wav(file_name.c_str(), &decoder_cfg, &decoder) != MA_SUCCESS) {
        
        std::cerr << "Could not open file " << file_name << ".wav\n";
    
    } else {
        
        ma_uint64 numFrames;
        ma_decoder_get_available_frames(&decoder, &numFrames);

        if (numFrames > 0) {
            data.resize(numFrames);
            ma_decoder_read_pcm_frames(&decoder, data.data(), numFrames);
        }
        
        ma_decoder_uninit(&decoder);
    
    }

    return data;

}