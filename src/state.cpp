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
#include <state.hpp>
ma_context              guitar_amp::state::c;
ma_device_info *        guitar_amp::state::outputDeviceInfo;
ma_device_info *        guitar_amp::state::inputDeviceInfo; 
ma_uint32               guitar_amp::state::numOutputDevices;
ma_uint32               guitar_amp::state::numInputDevices;
ma_uint32               guitar_amp::state::selectedInput;
ma_uint32               guitar_amp::state::selectedOutput;
ma_device_config        guitar_amp::state::deviceConf;
ma_device               guitar_amp::state::device;