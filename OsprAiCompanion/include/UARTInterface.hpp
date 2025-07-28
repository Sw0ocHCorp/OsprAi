#ifndef UART_INTERFACE_HPP
#define UART_INTERFACE_HPP
#include "ComInterface.hpp"
#include "Utils.hpp"

#include <string.h>
#include <wiringPi.h>
#include <wiringSerial.h>

class UARTInterface : public ComInterface
{
    private:
        char *PortPath;
        int BaudRate;
        int UARTPort;
        std::chrono::steady_clock::time_point CurrentTime;

    protected:
        void runTask() {
            bool isConnected = false;
            while(this->IsRunning) {
                if (!isConnected) {
                    isConnected = connect();
                    CurrentTime = std::chrono::steady_clock::now();
                }
                else {
                    listenForIncomingFrame();
                    if (chrono::steady_clock::now() - CurrentTime > chrono::milliseconds(200)) {
                        cout << "Sending frame..." << endl;
                        sendFrame("test\n");
                        CurrentTime = chrono::steady_clock::now();
                    }
                }
            }
        }
    public:
        UARTInterface(char *serialPath, int baudrate) {
            PortPath = serialPath;
            BaudRate = baudrate;
            this->startTask();
        }

        bool connect() {
            UARTPort = serialOpen(PortPath, BaudRate);
            if(UARTPort < 0) {
                cout << "Error opening UART port: " << PortPath << endl;
                return false;
            } else {
                cout << "UART port opened successfully: " << PortPath << endl;
                return true;
            }
        }

        bool sendFrame(string frame) {
            serialPuts(UARTPort, frame.c_str());
            return true;
        }

        void listenForIncomingFrame() {
            string data;
            while (serialDataAvail(UARTPort)) {
                char incomingChar= intToAsciiChar(serialGetchar(UARTPort));
                data.push_back(incomingChar);
                if (incomingChar == '\n') {
                    FrameReceivedEvent.trigger(data);
                    serialFlush(UARTPort); // Clear the serial buffer after reading
                }
            }
            
        }
};

#endif