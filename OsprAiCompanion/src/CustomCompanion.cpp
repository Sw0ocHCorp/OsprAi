#include "../include/UDPInterface.hpp"
#include "EventsManagement.hpp"
#include "UARTInterface.hpp"
#include "FrameParser.hpp"

int main() {
    FrameParser rcParser(StaticVector<uint8_t, 10> {0xAB, 0xCD}, StaticVector<StaticVector<uint8_t, 10>, 10> { StaticVector<uint8_t, 10> {0x00, 0x0A}, StaticVector<uint8_t, 10> {0x00, 0x0B} },
                                                                    StaticVector<StaticVector<char, 10>, 10> {StaticVector<char, 10> {'a','r','m','i','n','g'}, StaticVector<char, 10> {'s','t','i','c','k','s'}});

    FrameParser fcParser(StaticVector<uint8_t, 10>{0xAB, 0xCD}, StaticVector<StaticVector<uint8_t, 10>, 10> {StaticVector<uint8_t, 10> {0x00, 0x0F}, StaticVector<uint8_t, 10>{0x00, 0x10},
																												    StaticVector<uint8_t, 10>{0x00, 0x11}, StaticVector<uint8_t, 10>{0x00, 0x12}}, 
                                                                StaticVector<StaticVector<char, 10>, 10> {StaticVector<char, 10> {'l','i','n','s','p','d'}, StaticVector<char, 10> {'r','o','t','s','p','d'},
                                                                                                                    StaticVector<char, 10> {'a','l','t','i','t','d'}, StaticVector<char, 10> {'t','h','e','t','a'}});

    UDPInterface eth("192.168.1.221", 8080, "192.168.1.173", 8080, rcParser);
    UARTInterface uart((char *)"/dev/ttyAMA1", B921600, fcParser);
    Observer<StaticVector<uint8_t, 500>> ethObserver = Observer<StaticVector<uint8_t, 500>>();
    ethObserver.setCallback(std::bind(&UARTInterface::enqueueNewFrame, &uart, std::placeholders::_1));
    Observer<StaticVector<uint8_t, 500>> uartObserver = Observer<StaticVector<uint8_t, 500>>();
    uartObserver.setCallback(std::bind(&UDPInterface::enqueueNewFrame, &eth, std::placeholders::_1));
    eth.addFrameReceivedObserver(&ethObserver);
    uart.addFrameReceivedObserver(&uartObserver);
    //Maintain the main thread alive
    while(true) {

    }
    return 1;
}
//11+4+4+12+1