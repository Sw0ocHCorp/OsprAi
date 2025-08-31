#ifndef ETHERNET_INTERFACE_HPP
#define ETHERNET_INTERFACE_HPP
#include <sys/socket.h>
#include <netinet/in.h>
#include <string>
#include <iostream>
#include <arpa/inet.h>
#include <thread>
#include "ComInterface.hpp"

using namespace std;

class UDPInterface : public ComInterface
{
    private:
        string Address;
        string TargetAddress;
        int Port;
        int TargetPort;
        int Socket= -1;
        struct sockaddr_in Interface;
        struct sockaddr_in TargetInterface;

    protected:

        WorldMap dataFrameToWorldMap(StaticVector<StaticVector<float, 10>, 10> data) override {
            WorldMap map;
            
            
            return map;
        }

    public:
        UDPInterface(string address, int port, string targetAddress, int targetPort, FrameParser parser) : ComInterface(parser) {
            Address = address;
            Port = port;
            TargetAddress = targetAddress;
            TargetPort = targetPort;
            
            TargetInterface.sin_family = AF_INET;
            TargetInterface.sin_port = htons(targetPort);
            TargetInterface.sin_addr.s_addr = inet_addr(targetAddress.c_str());
            this->startTask();
        }

        void stopTask() override {
            ComInterface::stopTask();
            if (Socket >= 0) {
                Socket = -1;
                cout << "Stop Ethernet Task => Socket closed" << endl;
            }
        }

        bool connect() override {
            if (Socket < 0) {
                Socket= socket(AF_INET, SOCK_DGRAM, 0);
                if (Socket < 0) {
                    cout << "Error creating socket" << endl;
                    return false;
                }
            }
            Interface.sin_family = AF_INET;
            Interface.sin_port = htons(Port);
            Interface.sin_addr.s_addr = inet_addr(Address.c_str());
            if (bind(Socket, (struct sockaddr *)&Interface, sizeof(Interface)) < 0) {
                return false;
            }
            cout << "Socket bind successfully" << endl;
            cout << "Connected to " << Address << ":" << Port << endl;
            return true;
        }

        bool sendRawFrame(StaticVector<uint8_t, 500> rawFrame) override {
            socklen_t sockLen= sizeof(TargetInterface);
            int sentBytes = sendto(Socket, rawFrame.data(), rawFrame.size(), 0, (struct sockaddr *)&TargetInterface, sockLen);
            if (sentBytes < 0) {
                cout << "Error sending raw frame" << endl;
                return false;
            }
            return true;
        }

        StaticVector<uint8_t, 500> listenForIncomingFrame() override {
            StaticVector<uint8_t, 500> frame;
            socklen_t sockLen= sizeof(TargetInterface);
            uint8_t buffer[500];
            int receivedBytes = recvfrom(Socket, buffer, 500, 0, (struct sockaddr *)&TargetInterface, &sockLen);
            if (receivedBytes >= 0)  {
                buffer[receivedBytes] = '\0'; // Null-terminate the received data
                frame.Add(buffer, receivedBytes);
            }
            return frame;
        }

        string getAddress() const {
            return Address;
        }

        int getPort() const {
            return Port;
        }
};

#endif