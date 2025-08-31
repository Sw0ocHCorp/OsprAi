#ifndef UART_INTERFACE_HPP
#define UART_INTERFACE_HPP
#include "ComInterface.hpp"
#include <unistd.h>
#include <fcntl.h>
#include <termios.h>
#include <errno.h> // Error integer and strerror() function
#include <bits/stdc++.h>

class UARTInterface : public ComInterface
{
    private:
        int SerialPort;
        char *PortPath;
        long BaudRate;
        struct termios Tty;

    public:
        UARTInterface(char *serialPath, long baudrate, FrameParser parser) : ComInterface(parser) {
            PortPath = serialPath;
            BaudRate = baudrate;
            this->startTask();
        }

        bool connect() override {
            SerialPort = open(PortPath, O_RDWR | O_NOCTTY | O_NDELAY);
            if (SerialPort < 0) {
                cout << "Error Opening Serial Port" << endl;
                return false;
            } else {
                cout << "Serial Port OK" << endl;
                if(tcgetattr(SerialPort, &Tty) != 0) {
                    printf("Error %i from tcgetattr", errno);
                    return false;
                } else {
                    if (cfsetspeed(&Tty, BaudRate) < 0){
                        cout << "Wrong baudrate selection" << endl;
                        return false;
                    }
                    cfmakeraw(&Tty);
                    Tty.c_cc[VTIME] = 0;    
                    Tty.c_cc[VMIN] = 0;
                    if (tcsetattr(SerialPort, TCSANOW, &Tty)) 
                        cout << "Error setting Port Attributes" << endl;
                }
                cout << "Serial configuration OK" << endl;
                return true;
            }
        }

        bool sendRawFrame(StaticVector<uint8_t, 500> rawFrame) override {
            int nBytes= write(SerialPort, rawFrame.data(), rawFrame.size());
            if (nBytes > 0) {
                cout << "Send " << nBytes << " bytes" << endl;
                return true;
            }
            else
                return false;
        }

        StaticVector<uint8_t, 500> listenForIncomingFrame() override {
            StaticVector<uint8_t, 500> frame;
            uint8_t data[1];
            struct timeval start;
            struct timeval end;
            gettimeofday(&start, NULL);
            gettimeofday(&end, NULL);
            double cumulTime= 0.0;
            while (frame.size() < frame.GetMaxSize()) {
                gettimeofday(&start, NULL);
                int nBytes= read(SerialPort, data, 1);
                gettimeofday(&end, NULL);
                if (nBytes > 0) {
                    cumulTime= 0.0;
                    frame.Add(data[0]);
                } else {
                    cumulTime += (end.tv_usec - start.tv_usec);
                }
                if (cumulTime > 500.0 || (frame.size() >= 2 && frame[frame.size()-1] == '\n' && frame[frame.size()-2] == '\r'))
                    break;
            }
            return frame;
        }

        WorldMap dataFrameToWorldMap(StaticVector<StaticVector<float, 10>, 10> data) override {
            WorldMap map;
            StaticVector<StaticVector<char, 10>, 10> labels= Parser.getLabels();
            for(int i= 0; i < labels.size(); i++) {
                if (findPattern(labels[i].data(), labels[i].size(), "arm", 3)>= 0) 
                    map.ArmingState= (uint8_t)data[i][0];
                else if (findPattern(labels[i].data(), labels[i].size(), "lin", 3)>= 0) {
                    map.LinSpeed[0]= data[i][0];
                    map.LinSpeed[1]= data[i][1];
                    map.LinSpeed[2]= data[i][2];
                }
                else if (findPattern(labels[i].data(), labels[i].size(), "rot", 3)>= 0) {
                    map.RotSpeed[0]= data[i][0];
                    map.RotSpeed[1]= data[i][1];
                    map.RotSpeed[2]= data[i][2];
                }
                else if (findPattern(labels[i].data(), labels[i].size(), "alt", 3)>= 0) {
                    map.Altitude= data[i][0];
                }
                else if (findPattern(labels[i].data(), labels[i].size(), "theta", 5)>= 0) {
                    map.Theta= data[i][0];
                }
            }
            return map;
        }
};

#endif