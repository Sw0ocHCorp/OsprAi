# OsprAi

OsprAi is an Open Source Drone ecosystem.
It is composed of:
* Remote Controller Software in Rust
* Companion Software in C++
* Flight Controller Software in C++

For the begining of the project I build my own encoding system for the communication between all the softwares:

The SOF of ALL the Frames is "abcd"

## Low Level Data
| Data Label                    | Data ID   | Data                                          |
| :-------------:               | :-------: | :-------------:                               |
| Arming/Disarm                 | 000a      | Single value (Arm `0a` DisArm `0f`)           |
| Joystick                      | 000b      | 1 Value / Axis (2 Joystick, 4 Axis)           |
| Drone Speed Setpoint          | 000c      | 1 Value / Axis (3D Vector)                    |
| Drone Angles Setpoint         | 000d      | For the begining 1 Value                      |
| Servos Angles Setpoint        | 000e      | 1 Value / Servos                              |
| Drone Speed Mesurement        | 000f      | 1 Value Axis (Sensor Fusion, 2 IMUs: 3 Axis)  |
| Drone Gyro Mesurement         | 0010      | 1 Value Axis (Sensor Fusion, 2 IMUs: 3 Axis)  |
| Altitude Mesurement           | 0011      | 1 Value (Sensor Fusion, 2 Barometers: 1 Value)|
| GPS Lat/Lon Mesurement        | 0012      | Lat/Lon/                                      |
| GPS Speed Heading Mesurement  | 0013      | Speed()/Heading()                             |
| Lidar Measurement             | 0014      | 2 Values (ToF & Lidar Orientation via IMU)    |
| Battery Mesurement            | 0015      | 1 Value (Remain Energy)                       |
| Cam Image                     | 0016      |                                               |


## High Level Data
| Data Label            | Data ID           | Data                      |
| :-------------:       | :-------------:   | :-------------:           |


