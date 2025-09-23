#ifndef IMU_MANAGER_HPP
#define IMU_MANAGER_HPP

extern "C" {
#include <linux/i2c-dev.h>
#include <i2c/smbus.h>
}
#include <sys/ioctl.h>
#include <iostream>
#include <fcntl.h>
#include "EventsManagement.hpp"
#include "BasicInterfaces/I2CInterface.hpp"
#include "Utils.hpp"

#define GYRO_CONFIG_REG     0x1B
#define ACCEL_CONFIG_REG    0x1C
#define WAKE_UP_MODE_REG    0x6B
#define LIN_ACCEL_MES_REG   0x3B
#define GYRO_ACCEL_MES_REG   0x43       

enum GyroConfig {
    G250=       250,
    G500=       500, 
    G1000=      1000,
    G2000=      2000
};

enum AccelConfig {
    A4G=        4,
    A8G=        8,
    A16G=       16,
    A32G=       32
};

struct IMUConfig {
    GyroConfig GyroRange;
    AccelConfig AccelRange;
};

class IMUManager : public I2CInterface {
    protected:
        IMUConfig Config;
        Event<IMUData> DataStreamEvent;
        StaticVector<uint8_t, 10> SensorAddrs;
        bool IsRunning= false;

    public:
        IMUManager(StaticVector<char, 25> i2cPortPath, StaticVector<uint8_t, 10> sensorAddrs, IMUConfig sensorConfig) : I2CInterface(i2cPortPath) {
            SensorAddrs= sensorAddrs;
            Config= sensorConfig;
            if (I2CPort < 0) {
                cerr << "Error Opening " << i2cPortPath.data() << endl;
            }
            else {
                for(int i= 0; i < SensorAddrs.size(); i++) {
                    if (IsSlaveConnected(SensorAddrs[i]) == false) {
                        cerr << "Error Opening Device with Address: " << (int)SensorAddrs[i] << endl;
                    } else {
                        StaticVector<uint8_t, 25> data= readRegData(SensorAddrs[i], 0x75, 1);
                        if (data[0] == 0x70) {
                            //Sensor Configuration
                            uint8_t gyroConfig= 0;
                            uint8_t accelConfig= 0;
                            switch (Config.GyroRange)
                            {
                                case G250:
                                    gyroConfig= 0b00000000;
                                    break;
                                case G500:
                                    gyroConfig= 0b00001000;
                                    break;
                                case G1000:
                                    gyroConfig= 0b00010000;
                                    break;
                                case G2000:
                                    gyroConfig= 0b00011000;
                                    break;
                                default:
                                    break;
                            }
                            switch (Config.AccelRange)
                            {
                                case A4G:
                                    accelConfig= 0b00000000;
                                    break;
                                case A8G:
                                    accelConfig= 0b00001000;
                                    break;
                                case A16G:
                                    accelConfig= 0b00010000;
                                    break;
                                case A32G:
                                    accelConfig= 0b00011000;
                                    break;
                                default:
                                    break;
                            }
                            if (writeDataInReg(SensorAddrs[i], GYRO_CONFIG_REG, StaticVector<uint8_t, 10> {gyroConfig}) == false ||  
                                    writeDataInReg(SensorAddrs[i], ACCEL_CONFIG_REG, StaticVector<uint8_t, 10> {accelConfig}) == false ||
                                    writeDataInReg(SensorAddrs[i], WAKE_UP_MODE_REG, StaticVector<uint8_t, 10> {0}) == false) {
                                cerr << "Error in Sensor " << (int)SensorAddrs[i] << " configuration" << endl;
                            } else 
                                IsRunning= true;
                        }  
                    }
                }
            }
        }

        ~IMUManager() {

        }

    IMUData getMeasurements() {
        IMUData data;
        if (IsRunning) {
            StaticVector<float, 3> linAccelData= processRawData(readRegData(SensorAddrs[0], LIN_ACCEL_MES_REG, 6));
            StaticVector<float, 3> gyroAccelData= processRawData(readRegData(SensorAddrs[0], GYRO_ACCEL_MES_REG, 6));
            int a= 1;
        }
        return data;
    }

    StaticVector<float, 3> processRawData(StaticVector<uint8_t, 25> rawData) {
        StaticVector<float, 3> linAccelVec;
        for(int i= 0; i < linAccelVec.maxSize()*2; i += 2) {
            int16_t signAccel= ((int16_t)rawData[i] << 8) | (rawData[i+1] << 0);
            linAccelVec.add((float)signAccel / (pow((double)2, (double)16) / Config.AccelRange));
        }
        return linAccelVec;
    }

};

#endif