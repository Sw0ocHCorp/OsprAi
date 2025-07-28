#ifndef UTILS_HPP
#define UTILS_HPP
#include <vector>
#include <string>
#include <string.h>
#include <map>
#include <sstream>  
#include <iomanip>
#include <string>

using namespace std;

float hexStringToFloat(string hexString) {
    unsigned int hexValue;
    stringstream ss;
    ss << hex << hexString;
    ss >> hexValue;
    float floatValue;
    memcpy(&floatValue, &hexValue, sizeof(floatValue));
    return floatValue;
}

string floatToHexString(float value) {
    unsigned int hexValue;
    memcpy(&hexValue, &value, sizeof(value));
    stringstream ss;
    ss << hex << hexValue;
    return ss.str();
}

string uCharToHexString(unsigned char value) {
    stringstream ss;
    ss << std::hex << setw(2) << setfill('0') << static_cast<int>(value);
    return ss.str();
}

char intToAsciiChar(int value) {
    if (value < 0 || value > 255) {
        throw std::out_of_range("Value must be between 0 and 255");
    }
    return static_cast<char>(value);
}

#endif