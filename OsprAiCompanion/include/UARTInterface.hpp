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

    protected:
        void runTask() {
            bool isConnected = false;
            while(this->IsRunning) {
                if (!isConnected)
                    isConnected = connect();
                else {
                    char buffer[256];
                    std::memset(buffer, 0, sizeof(buffer) / sizeof(buffer[0]));
                    //bool isSend= sendFrame("debug");
                    if(listenForIncomingFrame(buffer, sizeof(buffer)/sizeof(buffer[0])) == true)
                        cout << "Data Received" << endl;
                        //isConnected = false;
                }
            }
        }
    public:
        UARTInterface(char *serialPath, long baudrate) {
            PortPath = serialPath;
            BaudRate = baudrate;
            this->startTask();
        }

        bool connect() {
            SerialPort = open(PortPath, O_RDWR | O_NOCTTY | O_NDELAY);
            if (SerialPort < 0) {
                cout << "Error Opening Serial Port" << endl;
                return false;
            } else {
                cout << "Serial Port OK" << endl;
                if(tcgetattr(SerialPort, &Tty) != 0) {
                    printf("Error %i from tcgetattr: %s\n", errno);
                    return false;
                } else {
                    if (cfsetspeed(&Tty, BaudRate) < 0){
                        cout << "Wrong baudrate selection" << endl;
                        return false;
                    }
                    cfmakeraw(&Tty);
                    if (tcsetattr(SerialPort, TCSANOW, &Tty)) 
                        cout << "Error setting Port Attributes" << endl;
                    Tty.c_cc[VTIME] = 0;    
                    Tty.c_cc[VMIN] = 0;
                }
                cout << "Serial configuration OK" << endl;
                return true;
            }
        }

        bool sendFrame(string frame) {
            if (write(SerialPort, frame.data(), frame.size())>= 0)
                return true;
            else
                return false;
        }

        bool listenForIncomingFrame(char *buffer, int bufferSize) {
            int nBytes= read(SerialPort, buffer, bufferSize);
            if (nBytes >= 0) {
                buffer[nBytes] = '\0';
                cout << buffer << endl;
                return true;
            }
            else    
                return false;
        }
};

#endif