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