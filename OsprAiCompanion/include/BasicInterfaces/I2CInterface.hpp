#ifndef I2C_INTERFACE_HPP
#define I2C_INTERFACE_HPP

extern "C" {
#include <linux/i2c.h>
#include <linux/i2c-dev.h>
}

#include "EventsManagement.hpp"
#include "Utils.hpp"
#include <sys/ioctl.h>
#include <iostream>
#include <fcntl.h>


class I2CInterface {
    protected:
        int I2CPort= -1;

    public:
        I2CInterface(StaticVector<char, 25> i2cPortPath) {
            I2CPort= open(i2cPortPath.data(), O_RDWR);
        }

        ~I2CInterface() {

        }

        bool IsSlaveConnected(uint8_t deviceAddr) {
            int fdbck= ioctl(I2CPort, I2C_SLAVE, deviceAddr);
            if (fdbck >= 0)
                return true;
            else
                return false;
        }

        bool askForData(uint8_t targetDeviceAddress, uint8_t dataRegister) {
            i2c_rdwr_ioctl_data transaction;
            i2c_msg msg;
            memset(&msg, 0, sizeof(msg));
            memset(&transaction, 0, sizeof(transaction));
            msg.addr= targetDeviceAddress;
            msg.buf= &dataRegister;
            msg.flags= 0;
            msg.len= 1;
            transaction.msgs= &msg;
            transaction.nmsgs= 1;
            if (ioctl(I2CPort, I2C_RDWR, &transaction) < 0) {
                return false;
            }
            return true;
        }

        bool writeDataInReg(uint8_t targetDeviceAddress, uint8_t dataRegister, StaticVector<uint8_t, 10> data) {
            data.insert(0, dataRegister);
            i2c_rdwr_ioctl_data transaction;
            i2c_msg msg;
            memset(&msg, 0, sizeof(msg));
            memset(&transaction, 0, sizeof(transaction));
            msg.addr= targetDeviceAddress;
            msg.buf= (unsigned char *)data.data();
            msg.flags= 0;
            msg.len= data.size();
            transaction.msgs= &msg;
            transaction.nmsgs= 1;
            if (ioctl(I2CPort, I2C_RDWR, &transaction) < 0) {
                return false;
            }
            return true;
        }

        StaticVector<uint8_t, 25> readRegData(uint8_t targetDeviceAddress, uint8_t dataRegister, int outputLength) {
            StaticVector<uint8_t, 25> data;
            uint8_t buff[25];
            if (askForData(targetDeviceAddress, dataRegister)) {
                i2c_rdwr_ioctl_data transaction;
                i2c_msg msg;
                memset(&msg, 0, sizeof(msg));
                memset(&transaction, 0, sizeof(transaction));
                msg.addr= targetDeviceAddress;
                msg.buf= buff;
                msg.flags= I2C_M_RD;
                msg.len= outputLength;
                transaction.msgs= &msg;
                transaction.nmsgs= 1;
                if (ioctl(I2CPort, I2C_RDWR, &transaction) >= 0) {
                    data.add(msg.buf, msg.len);
                }
            }
            return data;
        }
};
#endif

