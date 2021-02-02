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

#include "imgui.h"
#include "imgui-SFML.h"
#include "imnodes.h"

#define AMP_SAMPLE_RATE 48000

#include <internal_dsp.hpp>
#include <state.hpp>
using namespace guitar_amp;

void callback(ma_device *d, void* output, const void *input, ma_uint32 frameCount) {
    MA_ASSERT(d->capture.format == d->playback.format);
    MA_ASSERT(d->capture.channels == d->playback.channels);

    float *input_float = (float *) input;
    float *output_buf = (float *) malloc(frameCount * ma_get_bytes_per_frame(d->capture.format, d->capture.channels));
    guitar_amp::dsp::hardclip(input_float, output_buf, 0.01, frameCount);

    // tonestack simulation

    // put into real output
    MA_COPY_MEMORY(output, output_buf, frameCount * ma_get_bytes_per_frame(d->capture.format, d->capture.channels));
    free(output_buf);
}

int main () {

    /*
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
    */
    
    // Initialize ImGui
    sf::Event e;
    sf::RenderWindow w(sf::VideoMode(800,600), "Guitar Amp");
    sf::Clock dt;
    ImGui::SFML::Init(w);
    imnodes::Initialize();

    while (w.isOpen()) {
        while (w.pollEvent(e)) {
            ImGui::SFML::ProcessEvent(e);
            if (e.type == sf::Event::Closed) {
                imnodes::Shutdown();
                w.close();
            }
        }
        w.clear();
        ImGui::SFML::Update(w, dt.restart());

        // imgui stuff

        imnodes::BeginNodeEditor();

            std::string names[] = {"Input", "Preamp", "Output"};

            for (int i = 0; i < 3; i++) {
                imnodes::BeginNode(i);
                    imnodes::BeginNodeTitleBar();
                        ImGui::TextUnformatted(names[i].c_str());
                    imnodes::EndNodeTitleBar();
                imnodes::EndNode();
            }

        imnodes::EndNodeEditor();

        // raw sfml calls
        

        ImGui::SFML::Render(w);
        w.display();
    }

    // ma_device_uninit(&state::device);
    return 0;
}

