#define MINIAUDIO_IMPLEMENTATION
#include "miniaudio.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define AMP_SAMPLE_RATE 48000

ma_context              c;
ma_device_info *        outputDeviceInfo;
ma_device_info *        inputDeviceInfo; 
ma_uint32               numOutputDevices;
ma_uint32               numInputDevices;
ma_uint32               selectedInput;
ma_uint32               selectedOutput;
ma_device_config        deviceConf;
ma_device               device;

void list_devices(ma_device_info *deviceList, ma_uint32 n) {
    for (ma_uint32 i = 0; i < n; i++) {
        printf("%u. %s\n", i+1, deviceList[i].name);
    }
}

void hardclip(const float *input, float *transform, float threshold, ma_uint32 frameCount) {
    for (ma_uint32 i = 0; i < frameCount; i++) {
        if (input[i] > 0) {
            transform[i] = min(input[i]*10, threshold);
        } else {
            transform[i] = max(input[i]*10, -threshold);
        }
    }
}

void noisegate(const float *input, float *transform, float threshold, ma_uint32 frameCount, float transitionTime) {

}

void callback(ma_device *d, void* output, const void *input, ma_uint32 frameCount) {
    MA_ASSERT(d->capture.format == d->playback.format);
    MA_ASSERT(d->capture.channels == d->playback.channels);

    float *output_float = (float *) output;
    float *input_float = (float *) input;
    float *output_buf = malloc(sizeof(float)*frameCount);
    hardclip(input_float, output_buf, 0.05, frameCount);
    MA_COPY_MEMORY(output, output_buf, frameCount * ma_get_bytes_per_frame(d->capture.format, d->capture.channels));
}

int main () {

    if (ma_context_init(NULL, 0, NULL, &c) != MA_SUCCESS) {
       printf("Error initializing context\n");
       exit(-1); 
    }
    
    if (ma_context_get_devices(&c, &outputDeviceInfo, &numOutputDevices, &inputDeviceInfo, &numInputDevices) != MA_SUCCESS) {
        printf("Could not obtain audio device info!\n");
        exit(-1);
    }

    char response1[256];
    char response2[256];

    memset(response1, 0, 256);
    memset(response2, 0, 256);
    
    printf("Please select an output device [1-%u]\n", numOutputDevices);
    list_devices(outputDeviceInfo, numOutputDevices);
    
    gets(response1);
    selectedOutput=atoi(response1);
    printf("Please select an input device [1-%u]\n", numInputDevices);
    list_devices(inputDeviceInfo, numInputDevices);
    gets(response2);
    selectedInput=atoi(response2);

    if (selectedInput == 0 || selectedOutput == 0) {
        printf("Please select a valid device!\n");
        exit(-1);
    }

    selectedOutput--, selectedInput--;

    deviceConf = ma_device_config_init(ma_device_type_duplex);
    deviceConf.playback.format = ma_format_f32;
    deviceConf.capture.format = ma_format_f32;
    deviceConf.playback.pDeviceID = &(outputDeviceInfo[selectedOutput].id);
    deviceConf.capture.pDeviceID = &(inputDeviceInfo[selectedInput].id); 
    deviceConf.capture.channels = 1;
    deviceConf.playback.channels = 1;
    deviceConf.sampleRate = AMP_SAMPLE_RATE;
    deviceConf.dataCallback = callback;

    if (ma_device_init(&c, &deviceConf, &device) != MA_SUCCESS) {
        printf("Failed to access device\n");
        exit(-1);
    }

    ma_device_start(&device);
    printf("Press CTRL+C to exit\n");
    printf("Press ENTER to refresh configurations\n");
    float threshold = 0.5;

    while(1) {
        
    }
    ma_device_uninit(&device);
    return 0;
}

