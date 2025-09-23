#include "../include/UDPInterface.hpp"
#include "EventsManagement.hpp"
#include "UARTInterface.hpp"
#include "FrameParser.hpp"
#include "Actuators/ServosController.hpp"
#include "Sensors/IMUManager.hpp"

int main() {
    //We set 7bits addresses for I2C devices because ioctl function use 7 bits length addresses
    //Different from STM Nucleo because it use 8 bits addresses so we need to set DeviceAddress(7bits) << 1
    IMUConfig config;
    config.AccelRange= A16G;
    config.GyroRange= G2000;
    IMUManager imu(StaticVector<char, 25> ("/dev/i2c-1", 10), StaticVector<uint8_t, 10> {0x68}, config);
    ServosController servos(StaticVector<StaticVector<char, 50>, 4> {
                                StaticVector<char, 50> ("/sys/class/pwm/pwmchip0/pwm0", 28)
                            }, 0.025, 0.125, 0.005, 50, 270);


    /*FrameParser rcParser(StaticVector<uint8_t, 10> {0xAB, 0xCD}, StaticVector<StaticVector<uint8_t, 10>, 10> { StaticVector<uint8_t, 10> {0x00, 0x0A}, StaticVector<uint8_t, 10> {0x00, 0x0B} },
                                                                    StaticVector<StaticVector<char, 10>, 10> {StaticVector<char, 10> {'a','r','m','i','n','g'}, StaticVector<char, 10> {'s','t','i','c','k','s'}});

    FrameParser fcParser(StaticVector<uint8_t, 10>{0xAB, 0xCD}, StaticVector<StaticVector<uint8_t, 10>, 10> {StaticVector<uint8_t, 10> {0x00, 0x0F}, StaticVector<uint8_t, 10>{0x00, 0x10},
																												    StaticVector<uint8_t, 10>{0x00, 0x11}, StaticVector<uint8_t, 10>{0x00, 0x12}}, 
                                                                StaticVector<StaticVector<char, 10>, 10> {StaticVector<char, 10> {'l','i','n','s','p','d'}, StaticVector<char, 10> {'r','o','t','s','p','d'},
                                                                                                                    StaticVector<char, 10> {'a','l','t','i','t','d'}, StaticVector<char, 10> {'t','h','e','t','a'}});

    //FrameParser gpsParser(StaticVector<uint8_t, 10> {})
    UDPInterface eth("192.168.1.221", 8080, "192.168.1.173", 8080, rcParser);
    //UDPInterface eth("10.144.67.127", 8080, "10.144.67.73", 8080, rcParser);
    UARTInterface uart((char *)"/dev/ttyAMA1", B921600, fcParser);
    auto ethObserver= std::make_shared<Observer<StaticVector<uint8_t, 500>>>();
    //Observer<StaticVector<uint8_t, 500>> ethObserver = Observer<StaticVector<uint8_t, 500>>();
    ethObserver->setCallback(std::bind(&UARTInterface::enqueueNewFrame, &uart, std::placeholders::_1));
    //Observer<StaticVector<uint8_t, 500>> uartObserver = Observer<StaticVector<uint8_t, 500>>();
    auto uartObserver= std::make_shared<Observer<StaticVector<uint8_t, 500>>>();
    uartObserver->setCallback(std::bind(&UDPInterface::enqueueNewFrame, &eth, std::placeholders::_1));
    eth.addFrameReceivedObserver(ethObserver);
    uart.addFrameReceivedObserver(uartObserver);*/
    imu.getMeasurements();
    //Maintain the main thread alive
    struct timespec start, end;
    clock_gettime(CLOCK_MONOTONIC, &start);
    clock_gettime(CLOCK_MONOTONIC, &end);
    double elapsedTime= abs(end.tv_sec - start.tv_sec) * 1000.0
                                     + abs(end.tv_nsec - start.tv_nsec) / 1e6; 
    while(true) {
        clock_gettime(CLOCK_MONOTONIC, &end);
        elapsedTime= abs(end.tv_sec - start.tv_sec) * 1000.0
                                     + abs(end.tv_nsec - start.tv_nsec) / 1e6; 
        if (elapsedTime > 20) {
            clock_gettime(CLOCK_MONOTONIC, &start);
            servos.setAngleDutyCycle(180);
        }
    }
    return 1;
}
//11+4+4+12+1