#ifndef ETHERNET_INTERFACE_HPP
#define ETHERNET_INTERFACE_HPP
#include <sys/socket.h>
#include <netinet/in.h>
#include <string>
#include <iostream>
#include <arpa/inet.h>

using namespace std;

class EthernetInterface
{
    private:
        string Address;
        int Port;
        bool isClient;
        int Socket;
        struct sockaddr_in interfaceAddress;
    public:
        EthernetInterface(string address, int port, bool isClient= false) {

            Address = address;
            Port = port;
            this->isClient = isClient;
        }
        ~EthernetInterface(){

        }

        bool connect() {
            Socket= socket(AF_INET, SOCK_DGRAM, 0);
            if (Socket < 0) {
                cout << "Error creating socket" << endl;
                return false;
            }
            cout << "Socket created successfully" << endl;
            interfaceAddress.sin_family = AF_INET;
            interfaceAddress.sin_port = htons(Port);
            interfaceAddress.sin_addr.s_addr = inet_addr(Address.c_str());
            if (isClient == false) {
                if (bind(Socket, (struct sockaddr *)&interfaceAddress, sizeof(interfaceAddress)) < 0) {
                    cout << "Error binding socket" << endl;
                    return false;
                }
                cout << "Socket bind successfully" << endl;
            }
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

        bool listenForIncomingFrame(string targetIpAddress, int targetPort) {
            char buffer[1024];
            struct sockaddr_in sourceAddress;
            socklen_t addrLen = sizeof(sourceAddress);
            int receivedBytes = recvfrom(Socket, buffer, sizeof(buffer), 0, (struct sockaddr *)&sourceAddress, &addrLen);
            if (receivedBytes < 0) {
                cout << "Error receiving frame" << endl;
                return false;
            }
            buffer[receivedBytes] = '\0'; // Null-terminate the received data
            cout << "Frame received successfully from " << inet_ntoa(sourceAddress.sin_addr) << endl;
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