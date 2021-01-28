/*
    Real time guitar amplifier simulator
    Copyright (C) 2021  Nathaniel D'Rozario

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License along
    with this program; if not, write to the Free Software Foundation, Inc.,
    51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
*/
#define MINIAUDIO_IMPLEMENTATION

extern "C" {
#include "miniaudio.h"
}

#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <cmath>
#include <algorithm>
#include <SFML/Graphics.hpp>
#define NK_INCLUDE_FIXED_TYPES
#define NK_INCLUDE_STANDARD_IO
#define NK_INCLUDE_STANDARD_VARARGS
#define NK_INCLUDE_DEFAULT_ALLOCATOR
#define NK_INCLUDE_VERTEX_BUFFER_OUTPUT
#define NK_INCLUDE_FONT_BAKING
#define NK_INCLUDE_DEFAULT_FONT
#include "nuklear.h"
#include "nuklear_sfml.hpp"

#include <kfr/base.hpp>
#include <kfr/dsp.hpp>

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
            transform[i] = std::min(input[i]*10, threshold);
        } else {
            transform[i] = std::max(input[i]*10, -threshold);
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
    float *output_buf = (float *) malloc(frameCount * ma_get_bytes_per_frame(d->capture.format, d->capture.channels));
    hardclip(input_float, output_buf, 0.01, frameCount);

    // tonestack simulation 
    
    ma_loshelf2_config lowShelfConfig = ma_loshelf2_config_init(d->capture.format, d->capture.channels, d->sampleRate, -10, 5, 4000);
    ma_loshelf2 lowshelf;

    if (ma_loshelf2_init(&lowShelfConfig, &lowshelf) != MA_SUCCESS) {
        printf("cant init low shelf\n");
        exit(-1);
    }

    ma_loshelf2_process_pcm_frames(&lowshelf, output_buf, output_buf, frameCount);
    
    // put into real output
    MA_COPY_MEMORY(output, output_buf, frameCount * ma_get_bytes_per_frame(d->capture.format, d->capture.channels));
    free(output_buf);
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

