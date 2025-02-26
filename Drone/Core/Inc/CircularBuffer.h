/*
 * CircularBuffer.h
 *
 *  Created on: Feb 14, 2025
 *      Author: nclsr
 */

#ifndef INC_CIRCULARBUFFER_H_
#define INC_CIRCULARBUFFER_H_

#include "main.h"
#include <vector>

using namespace std;

class CircularBuffer {
private:
	vector<vector<uint8_t>> buffer;
	int head= 0;
	int tail= 0;
	int currentSize= 0;
	int maxSize;
public:
	CircularBuffer(int maxSize);
	virtual ~CircularBuffer();
	HAL_StatusTypeDef Enqueue(vector<uint8_t> frame);
	vector<uint8_t> Dequeue();
	int getCurrentSize();
	int getRemainSize();
};

#endif /* INC_CIRCULARBUFFER_H_ */
