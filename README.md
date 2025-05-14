# CodeCell Head / Body Tracker

The aim of this project is to make use of the capabilities of the miniature ESP32-C3 development board [CodeCell](https://microbots.io/pages/learn-codecell) by Microbots for computer music / spatial audio purposes.

The finished product should be a universal tool, that can be used either as a head tracker for binaural audio or as an expressive controller for virtual musical instruments or other computer music systems.

Motion and proximity data (from the BNO085 9-axis motion fusion sensor and the VCNL4040 IR light proximity sensor) are transmitted via OSC over WiFi using the [OSC_networking](https://git.iem.at/uC/osc_networking) library.

## Functionality (TODO)
- [x] pitch/yaw/roll angles for head / body tracking  
- [x] proximity sensing to be used as an additional control (usable range of ~15 cm)  
- [ ] process accelerometer data to sense motion gestures  
- [ ] explore further usable sensor data  
- [ ] fine tune OSC communication (so it can be used with e.g. [IEM plugins SceneRotator](https://plugins.iem.at/docs/osc/#scenerotator))  
- [ ] implement sleep / wake function  
- [ ] make 3D-printed enclosure

## Further development ideas
- implement [Head Tracker HID protocol](https://source.android.com/docs/core/interaction/sensors/head-tracker-hid-protocol)