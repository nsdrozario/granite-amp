#include <console_ui.hpp>

void list_devices(ma_device_info *deviceList, ma_uint32 n) {
    for (ma_uint32 i = 0; i < n; i++) {
        printf("%u. %s\n", i+1, deviceList[i].name);
    }
}

void consoleInputPrompt() {
    char response1[256];
    char response2[256];

    memset(response1, 0, 256);
    memset(response2, 0, 256);
    using guitar_amp::state;
    printf("Please select an output device [1-%u]\n", state::numOutputDevices);
    list_devices(state::outputDeviceInfo, state::numOutputDevices);
    
    gets(response1);
    state::selectedOutput=atoi(response1);
    printf("Please select an input device [1-%u]\n", state::numInputDevices);
    list_devices(state::inputDeviceInfo, state::numInputDevices);
    gets(response2);
    state::selectedInput=atoi(response2);

    if (state::selectedInput == 0 || state::selectedOutput == 0) {
        printf("Please select a valid device!\n");
        exit(-1);
    }
}
