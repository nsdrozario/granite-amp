/*
    Real time guitar amplifier simulation
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
