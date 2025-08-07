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

using namespace std;

bool Equal(vector<uint8_t> target, vector<uint8_t> pattern) {
	for(int i= 0; i < (int)pattern.size(); i++) {
		if (target.at(i) != pattern.at(i))
			return false;
	}
	return true;
}

int FindPattern(vector<uint8_t> targetStr, vector<uint8_t> pattern) {
	int index= -1;
	for (int i= 0; i < (int)targetStr.size(); i++) {
		vector<uint8_t> potentialPattern;
		for (int j= i; j < min((int)targetStr.size() ,i + (int)pattern.size()); j++) {
			potentialPattern.push_back(targetStr.at(j));
		}
		if (Equal(potentialPattern, pattern)) {
			index= i;
		}
	}
	return index;
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

vector<float> SortVec(vector<float> vec) {
	for(int i= 0; i < (int)vec.size(); i++) {
		for (int j= i; j < (int)vec.size(); j++) {
			//Si suivant inférieur au précédent, on interverti les données
			if (vec.at(j) < vec.at(i)) {
				float swapVal= vec.at(i);
				vec[i]= vec.at(j);
				vec[j]= swapVal;
			}
		}
	}
	return vec;
}


float Median(vector<float> vec) {
	vec= SortVec(vec);
	if (vec.size() %2 != 0) {
		return vec.at(vec.size()/2);
	}
	else {
		return (vec.at(vec.size()/2) + vec.at(vec.size()/2+1)) / 2;
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
