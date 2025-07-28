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
	CircularBuffer(int maxSize) {
		// TODO Auto-generated constructor stub
		this->maxSize= maxSize;
	}

	virtual ~CircularBuffer() {
		// TODO Auto-generated destructor stub
	}

	HAL_StatusTypeDef Enqueue(vector<uint8_t> frame) {
		//If we it remain space for incoming Frame, we store this Frame
		if (getRemainSize() > 0) {
			this->buffer.at(this->head) = frame;
			this->head++;
			this->currentSize++;
		}
		//If at the end of the buffer, go to the begining
		if (this->head >= this->maxSize) {
			this->head= 0;
		}
	}

	vector<uint8_t> Dequeue() {
		vector<uint8_t> frame;
		//If the buffer is not empty, we take the first frame we have access;
		if (this->currentSize > 0) {
			frame= this->buffer.at(this->tail);
			this->tail++;
			this->currentSize--;
		}
		//If at the end of the buffer, go to the begining
		if (this->tail >= this->maxSize) {
			this->tail = 0;
		}
		return frame;
	}
	int getCurrentSize() { return this->currentSize; }
	int getRemainSize() { return this->maxSize - this->currentSize; }
};

#endif /* INC_CIRCULARBUFFER_H_ */
