/*
 * utils.h
 *
 *  Created on: Feb 15, 2025
 *      Author: nclsr
 */

#ifndef INC_UTILS_H_
#define INC_UTILS_H_

#include "main.h"
#include <vector>
#include <cstdint>
#include <map>
#include <math.h>
#include <string>
#include <string.h>
#include <sstream>
#include <iomanip>
#include <iostream>
#include <memory>
#include <algorithm>

using namespace std;

template<typename T, unsigned int MS>
class StaticVector  {
	private:
		T Data[MS];
		unsigned int MaxSize= MS;
		unsigned int Size= 0;
	public:
		StaticVector() {

		}

		StaticVector(std::initializer_list<T> init) {
			if ((unsigned int)init.size() <= MaxSize) {
				copy(init.begin(), init.end(), Data);
				Size= init.size();
			}
		}
		void Add(T data) {
			if (Size < MaxSize) {
				Data[Size]= data;
				Size++;
			} else {
				throw std::out_of_range("Size");
			}
		}

		void Add(T *data, unsigned int n) {
			if (Size + (n-1) < MaxSize) {
				for (int i= 0; i < (int)n; i++) {
					Data[Size]= data[i];
					Size++;
				}
			} else {
				throw std::out_of_range("Size");
			}
		}

		bool Remove(T data) {
			int index= -1;
			for (int i= 0; i < Size; i++) {
				if (Data[i] == data) {
					index = i;
					break;
				}
			}
			if (index >= 0) {
				Size--;
				for (int i= index +1; index <= Size; i++) {
					Data[i-1]= Data[i];
				}
				return true;
			}
			return false;
		}
		void Clear() {
			Size= 0;
		}
		T& operator[](int index) {
			return Data[index];
		}
		const T *GetData() {
			return Data;
		}
		int GetSize() const {
			return Size;
		}
		int GetMaxSize() {
			return MaxSize;
		}
};

bool Equal(vector<uint8_t> target, vector<uint8_t> pattern) {
	if (target.size() < pattern.size()) {
		return false;
	} else {
		for(int i= 0; i < (int)pattern.size(); i++) {
			if (target.at(i) != pattern.at(i))
				return false;
		}
		return true;
	}
}

bool Equal(uint8_t *target, int targetSize, uint8_t *pattern, int patternSize) {
	if (targetSize < patternSize) {
		return false;
	} else {
		for(int i= 0; i < patternSize; i++) {
			if (target[i] != pattern[i])
				return false;
		}
		return true;
	}
}

int FindPattern(vector<uint8_t> targetStr, vector<uint8_t> pattern, bool findFromEnd= false) {
	int index= -1;
	int i= 0;
	if (targetStr.size() >= pattern.size()) {
		if (findFromEnd) {
			for (int i= (int)targetStr.size() - 1; i >= 0; i--) {
				vector<uint8_t> potentialPattern;
				for (int j= i; j < min((int)targetStr.size() ,i + (int)pattern.size()); j++) {
					potentialPattern.push_back(targetStr.at(j));
				}
				if (Equal(potentialPattern, pattern)) {
					index= i;
					return i;
				}
			}
		} else {
			for (int i= 0; i < (int)targetStr.size(); i++) {
				vector<uint8_t> potentialPattern;
				for (int j= i; j < min((int)targetStr.size() ,i + (int)pattern.size()); j++) {
					potentialPattern.push_back(targetStr.at(j));
				}
				if (Equal(potentialPattern, pattern)) {
					index= i;
					return i;
				}
			}
		}
	}
	return index;
}

int FindPattern(uint8_t *targetStr, int targetStrSize, uint8_t *pattern, int patternSize, bool findFromEnd= false) {
	int index= -1;
	if (targetStrSize >= patternSize) {
		if (findFromEnd) {
			for (int i= (int)targetStrSize - 1; i >= 0; i--) {
				vector<uint8_t> potentialPattern;
				for (int j= i; j < min((int)targetStrSize ,i + patternSize); j++) {
					potentialPattern.push_back(targetStr[j]);
				}
				if (Equal(potentialPattern.data(), potentialPattern.size(), pattern, patternSize)) {
					index= i;
					return i;
				}
			}
		} else {
			for (int i= 0; i < targetStrSize; i++) {
				vector<uint8_t> potentialPattern;
				for (int j= i; j < min((int)targetStrSize ,i + patternSize); j++) {
					potentialPattern.push_back(targetStr[j]);
				}
				uint8_t *test = potentialPattern.data();
				if (Equal(potentialPattern.data(), potentialPattern.size(), pattern, patternSize)) {
					index= i;
					return i;
				}
			}
		}
	}
	return index;
}

bool IsEOP(uint8_t *packet, int packetSize, uint8_t eopValue) {
	return FindPattern(packet, packetSize, new uint8_t[1]{eopValue}, 1, true);
}

double LinearInInterval(double value, double minSource, double maxSource, double minTarget, double maxTarget) {
	return ((value - minSource) / (maxSource - minSource)) * (maxTarget - minTarget) + minTarget;
}

uint8_t CharToByte(uint8_t charAsciiCode) {
	uint8_t realHexaValue= charAsciiCode;
	if (realHexaValue >= '0' && realHexaValue <= '9')
		realHexaValue -= '0';
	else if (realHexaValue >= 'A' && realHexaValue <= 'F')
		realHexaValue -= 'A' - 10;
	else if (realHexaValue >= 'a' && realHexaValue <= 'f')
		realHexaValue -= 'a' - 10;
	return realHexaValue;
}

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
    return static_cast<char>(value);
}

float Median(vector<float> vec) {
	int n= vec.size();
	sort(vec.begin(), vec.end());
	if (n %2 != 0) {
		return vec[n/2];
	}
	else {
		return (vec[n/2] + vec[(n-1)/2]) / 2;
	}
}

float Median(float *data, int n) {
	vector<float> vec(data, data+n);
	sort(vec.begin(), vec.end());
	if (n %2 != 0) {
		return vec[n/2];
	}
	else {
		return (vec[n/2] + vec[(n-1)/2]) / 2;
	}
}

void VecMin(vector<int> data, int *minValue, int *minIndex) {
	*minValue= data.at(0);
	*minIndex = 0;
	for (int i= 0; i < (int)data.size(); i++) {
		if (data.at(i) < *minValue) {
			*minValue = data.at(i);
			*minIndex= i;
		}
	}
}

void VecMax(vector<int> data, int *maxValue, int *maxIndex) {
	*maxValue= data.at(0);
	*maxIndex = 0;
	for (int i= 0; i < (int)data.size(); i++) {
		if (data.at(i) > *maxValue) {
			*maxValue = data.at(i);
			*maxIndex= i;
		}
	}
}

float Deg2Rad(float degAngle, bool needModulo) {
	float radAngle= (M_PI/180.0)*degAngle;
	if (needModulo)
		radAngle= fmod(radAngle, M_PI);
	return radAngle;
}

float Rad2Deg(float radAngle) {
	return (180.0/M_PI)*radAngle;
}

#endif /* INC_UTILS_H_ */
