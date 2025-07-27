#ifndef UTILS_HPP
#define UTILS_HPP
#include <vector>
#include <string>
#include <map>

using namespace std;

char intToAsciiChar(int value) {
    if (value < 0 || value > 255) {
        throw std::out_of_range("Value must be between 0 and 255");
    }
    return static_cast<char>(value);
}

#endif