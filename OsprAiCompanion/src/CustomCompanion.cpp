#include "../include/UDPInterface.hpp"
#include "EventsManagement.hpp"
#include "UARTInterface.hpp"
#include "FrameParser.hpp"

int main() {
    FrameParser parser("abcd", {{"0011", "test0"}, {"00a1", "test1"},{"0044", "test2"},{"00ce", "id3"}}, {{"encod0", "00f1"}, {"encod1", "00a2"}, {"encod2", "00b3"}, {"encod3", "00c4"}});
    FrameParser parser2("abcd", {{"00f1", "encod0"}, {"00a2", "encod1"}, {"00b3", "encod2"}, {"00c4", "encod3"}}, {{"0011", "test0"}, {"00a1", "test1"},{"0044", "test2"},{"00ce", "id3"}});
    /*string frame = "abcd26001102000a00a1044020000000440440b6666600ce0c4144cccd4205333342913333a8";
    parser.parseFrame(frame);*/
    string encodString= parser.encodeFrame({
        {"encod0", {1.2f, 2.1f, 3.6f}},
        {"encod1", {4.5f, 5.9f}},
        {"encod2", {6.7f, 7.8f, 8.9f}},
        {"encod3", {9.0f}}
    });
    cout << "Encoded Frame: " << encodString << endl;
    map<string, vector<float>> parsedData = parser2.parseFrame(encodString);
    cout << "Parsed Data: " << endl;
    for (const auto& [id, values] : parsedData) {
        cout << "  " << id << ": ";
        for (float value : values) {
            cout << value << " ";
        }
        cout << endl;
    }
    /*UDPInterface eth("192.168.1.109", 8080, "192.168.1.235", 8080);
    UARTInterface uart("/dev/ttyAMA0", 115200);
    FrameReceivedObserver observer = FrameReceivedObserver();
    eth.addFrameReceivedObserver(&observer);
    uart.addFrameReceivedObserver(&observer);
    //Maintient en vie du Soft
    while(true) {

    }*/
    return 1;
}
//11+4+4+12+1