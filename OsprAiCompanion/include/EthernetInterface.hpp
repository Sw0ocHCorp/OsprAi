#ifndef ETHERNET_INTERFACE_HPP
#define ETHERNET_INTERFACE_HPP
#include <sys/socket.h>
#include <netinet/in.h>
#include <string>
#include <iostream>
#include <arpa/inet.h>
#include <thread>
#include <EventsManagement.hpp>

using namespace std;

class EthernetInterface
{
    private:
        string Address;
        string TargetAddress;
        int Port;
        int TargetPort;
        int Socket;
        struct sockaddr_in InterfaceAddress;
        thread Task;
        bool IsRunning = false;
        Event<string> FrameReceivedEvent;
    public:
        EthernetInterface(string address, int port, string targetAddress, int targetPort) {
            Address = address;
            Port = port;
            TargetAddress = targetAddress;
            TargetPort = targetPort;
            IsRunning = true;
            Task = thread(&EthernetInterface::runTask, this);
        }
        ~EthernetInterface(){
            stopTask();
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
                else if (listenForIncomingFrame(TargetAddress, TargetPort, buffer, sizeof(buffer))) {
                    if (buffer[0] != '\0') {
                        FrameReceivedEvent.trigger(string(buffer));
                        cout << "Received frame: " << buffer << endl;
                    }
                    // Process the received frame as needed
                } else {
                    cout << "Failed to listen for incoming frame" << endl;
                }
            }
        }

        void stopTask() {
            this->IsRunning = false;
            if (Task.joinable()) {
                Task.join();
            }
            if (Socket >= 0) {
                Socket = -1;
                cout << "Stop Ethernet Task => Socket closed" << endl;
            }
        }

        bool connect() {
            Socket= socket(AF_INET, SOCK_DGRAM, 0);
            if (Socket < 0) {
                cout << "Error creating socket" << endl;
                return false;
            }
            cout << "Socket created successfully" << endl;
            InterfaceAddress.sin_family = AF_INET;
            InterfaceAddress.sin_port = htons(Port);
            InterfaceAddress.sin_addr.s_addr = inet_addr(Address.c_str());
            if (bind(Socket, (struct sockaddr *)&InterfaceAddress, sizeof(InterfaceAddress)) < 0) {
                cout << "Error binding socket" << endl;
                return false;
            }
            cout << "Socket bind successfully" << endl;
            return true;
        }

        bool sendFrame(string targetIpAddress, int targetPort, string frame) {
            struct sockaddr_in targetAddress;
            targetAddress.sin_family = AF_INET;
            targetAddress.sin_port = htons(targetPort);
            targetAddress.sin_addr.s_addr = inet_addr(targetIpAddress.c_str());
            int sentBytes = sendto(Socket, frame.c_str(), frame.size(), 0, (struct sockaddr *)&targetAddress, sizeof(targetAddress));
            if (sentBytes < 0) {
                cout << "Error sending frame" << endl;
                return false;
            }
            cout << "Frame sent successfully" << endl;
            return true;
        }

        bool listenForIncomingFrame(string targetIpAddress, int targetPort, char *buffer, int maxSize) {
            struct sockaddr_in sourceAddress;
            socklen_t addrLen = sizeof(sourceAddress);
            int receivedBytes = recvfrom(Socket, buffer, maxSize, 0, (struct sockaddr *)&sourceAddress, &addrLen);
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

        void addFrameReceivedObserver(Observer<string> *observer) {
            FrameReceivedEvent.addObserver(observer);
        }
};

#endif