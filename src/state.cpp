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