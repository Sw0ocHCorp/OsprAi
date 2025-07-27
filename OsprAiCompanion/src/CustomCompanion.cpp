#include "../include/UDPInterface.hpp"
#include "EventsManagement.hpp"
#include "UARTInterface.hpp"

int main() {
    UDPInterface eth("192.168.1.109", 8080, "192.168.1.235", 8080);
    UARTInterface uart("/dev/ttyAMA0", 115200);
    FrameReceivedObserver observer = FrameReceivedObserver();
    eth.addFrameReceivedObserver(&observer);
    uart.addFrameReceivedObserver(&observer);
    //Maintient en vie du Soft
    while(true) {

    }
    return 1;
}
