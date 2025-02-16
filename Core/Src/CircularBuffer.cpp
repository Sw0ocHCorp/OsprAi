/*
 * CircularBuffer.cpp
 *
 *  Created on: Feb 14, 2025
 *      Author: nclsr
 */

#include <CircularBuffer.h>

CircularBuffer::CircularBuffer(int maxSize) {
	// TODO Auto-generated constructor stub
	this->maxSize= maxSize;
}

CircularBuffer::~CircularBuffer() {
	// TODO Auto-generated destructor stub
}


HAL_StatusTypeDef CircularBuffer::Enqueue(vector<uint8_t> frame) {
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
vector<uint8_t> CircularBuffer::Dequeue() {
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
int CircularBuffer::getCurrentSize() {
	return this->currentSize;
}
int CircularBuffer::getRemainSize() {
	return this->maxSize - this->currentSize;
}
