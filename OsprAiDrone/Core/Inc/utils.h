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

using namespace std;

void NonBlockingDelay(int delayMs);
int FindPattern(vector<uint8_t> targetStr, vector<uint8_t> pattern);
bool Equal(vector<uint8_t> target, vector<uint8_t> pattern);
double LinearInInterval(double value, double minSource, double maxSource, double minTarget, double maxTarget) ;
uint8_t CharToByte(uint8_t charAsciiCode);

#endif /* INC_UTILS_H_ */
