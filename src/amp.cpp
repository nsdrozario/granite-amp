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
#include <miniaudio.h>
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
#include <nuklear.h>
#include <nuklear_sfml.hpp>

#define KFR_ENABLE_WAV 1
#include <kfr/all.hpp>

#define AMP_SAMPLE_RATE 48000

#include <internal_dsp.hpp>
#include <state.hpp>
using namespace guitar_amp;


void callback(ma_device *d, void* output, const void *input, ma_uint32 frameCount) {
    MA_ASSERT(d->capture.format == d->playback.format);
    MA_ASSERT(d->capture.channels == d->playback.channels);

    float *output_float = (float *) output;
    float *input_float = (float *) input;
    float *output_buf = (float *) malloc(frameCount * ma_get_bytes_per_frame(d->capture.format, d->capture.channels));
    guitar_amp::dsp::hardclip(input_float, output_buf, 0.01, frameCount);

    // tonestack simulation

    // put into real output
    MA_COPY_MEMORY(output, output_buf, frameCount * ma_get_bytes_per_frame(d->capture.format, d->capture.channels));
    free(output_buf);
}

int main () {

    // initialize miniaudio
    if (ma_context_init(NULL, 0, NULL, &state::c) != MA_SUCCESS) {
       printf("Error initializing context\n");
       exit(-1); 
    }
    
    if (ma_context_get_devices(&state::c, &state::state::outputDeviceInfo, &state::numOutputDevices, &state::inputDeviceInfo, &state::numInputDevices) != MA_SUCCESS) {
        printf("Could not obtain audio device info!\n");
        exit(-1);
    }

    state::selectedOutput--, state::selectedInput--;
    state::state::deviceConf = ma_device_config_init(ma_device_type_duplex);
    state::state::deviceConf.playback.format = ma_format_f32;
    state::deviceConf.capture.format = ma_format_f32;
    state::deviceConf.playback.pDeviceID = &(state::outputDeviceInfo[state::selectedOutput].id);
    state::deviceConf.capture.pDeviceID = &(state::inputDeviceInfo[state::selectedInput].id); 
    state::deviceConf.capture.channels = 1;
    state::deviceConf.playback.channels = 1;
    state::deviceConf.sampleRate = AMP_SAMPLE_RATE;
    state::deviceConf.dataCallback = callback;
    
    if (ma_device_init(&state::c, &state::state::deviceConf, &state::device) != MA_SUCCESS) {
        printf("Failed to access device\n");
        exit(-1);
    }

    // start amp simulation
    // ma_device_start(&state::device);
    printf("Press CTRL+C to exit\n");
    printf("Press ENTER to refresh configurations\n");

    

    // ma_device_uninit(&state::device);
    return 0;
}

