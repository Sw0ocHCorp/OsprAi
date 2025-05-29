#include "../include/EthernetInterface.hpp"

int main() {
    EthernetInterface eth("127.0.0.1", 8080);
    if (eth.connect()) {
        cout << "Connected successfully!" << endl;
        if (eth.sendFrame("127.0.0.1", 8081, "Hello, World!")) {
            cout << "Frame sent successfully!" << endl;
        } else {
            cout << "Failed to send frame!" << endl;
        }
    } else {
        cout << "Connection failed!" << endl;
    }
}
