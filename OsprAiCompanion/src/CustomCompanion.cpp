#include "../include/EthernetInterface.hpp"
#include "EventsManagement.hpp"

int main() {
    EthernetInterface eth("192.168.1.109", 8080, "192.168.1.235", 8080);
    FrameReceivedObserver observer = FrameReceivedObserver();
    eth.addFrameReceivedObserver(&observer);

    //Maintient en vie du Soft
    while(true) {

    }
}
