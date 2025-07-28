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
}

vector<double> SortVec(vector<double> vec) {
	for(int i= 0; i < vec.size(); i++) {
		for (int j= i; j < vec.size(); j++) {
			//Si suivant inférieur au précédent, on interverti les données
			if (vec.at(j) < vec.at(i)) {
				double swapVal= vec.at(i);
				vec[i]= vec.at(j);
				vec[j]= swapVal;
			}
		}
	}
	return vec;
}


double Median(vector<double> vec) {
	vec= SortVec(vec);
	if (vec.size() %2 != 0) {
		return vec[vec.size()/2];
	}
	else {
		return (vec[vec.size()/2] + vec[vec.size()/2+1]) / 2;
	}
}

class FrameParser {

};

#endif /* INC_UTILS_H_ */
