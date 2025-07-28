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

        void runTask() {
            bool isConnected = false;
            while(this->IsRunning) {
                char buffer[1024];
                if (!isConnected) {
                    isConnected = connect();
                    if (isConnected) {
                        cout << "Connected to " << Address << ":" << Port << endl;
                    }
                }
                else if (listenForIncomingFrame(buffer, sizeof(buffer))) {
                    if (buffer[0] != '\0') {
                        map<string, vector<float>> data= Parser.parseFrame(string(buffer)); 
                        FrameReceivedEvent.trigger(string(buffer));
                        cout << "Received frame: " << buffer << endl;
                    }
                    // Process the received frame as needed
                } else {
                    cout << "Failed to listen for incoming frame" << endl;
                }
            }
        }

        void stopTask() override {
            this->IsRunning = false;
            thread &task= getTask();
            if (task.joinable()) {
                task.join();
            }
            if (Socket >= 0) {
                Socket = -1;
                cout << "Stop Ethernet Task => Socket closed" << endl;
            }
        }

        bool connect() {
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
            return true;
        }

        bool sendFrame(string frame) {
            socklen_t sockLen= sizeof(TargetInterface);
            int sentBytes = sendto(Socket, frame.c_str(), frame.size(), 0, (struct sockaddr *)&TargetInterface, sockLen);
            if (sentBytes < 0) {
                cout << "Error sending frame" << endl;
                return false;
            }
            cout << "Frame sent successfully" << endl;
            return true;
        }

        bool listenForIncomingFrame(char *buffer, int maxSize) {
            socklen_t sockLen= sizeof(TargetInterface);
            int receivedBytes = recvfrom(Socket, buffer, maxSize, 0, (struct sockaddr *)&TargetInterface, &sockLen);
            if (receivedBytes < 0) {
                cout << "Error receiving frame" << endl;
                return false;
            }
            buffer[receivedBytes] = '\0'; // Null-terminate the received data
            //cout << "Frame received successfully from " << inet_ntoa(sourceAddress.sin_addr) << endl;
            return true;
        }

        string getAddress() const {
            return Address;
        }

        int getPort() const {
            return Port;
        }
};

#endif