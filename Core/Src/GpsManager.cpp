/*
 * GpsManager.cpp
 *
 *  Created on: Jan 18, 2025
 *      Author: nclsr
 */

#include <GpsManager.h>
#include "stdio.h"

namespace Osprai {

GpsManager::GpsManager() {
	// TODO Auto-generated constructor stub

}

//Manager Init Function
//separator: Sensor frame data separator
HAL_StatusTypeDef GpsManager::InitGps(UART_HandleTypeDef *bus, uint8_t sof , uint8_t separator) {
	this->sensorBus= bus;
	this->sof= sof;
	this->separator= separator;
	this->buffer= new uint8_t[128];
	memset(this->buffer, 0, 128);
}

double GpsManager::ParseMagneticVar(string *buff) {
	int floatPtIndex= buff->find('.');
	string leftSec= buff->substr(0, floatPtIndex);
	string rightSec= buff->substr(floatPtIndex+1, (buff->length()-3) - floatPtIndex);
	double sign= 1.0;
	if(buff->c_str()[buff->length()-1] == 'W')
		sign= -1.0;
	return stoi(leftSec, nullptr, 10) + (stoi(rightSec, nullptr, 10)/1000.0) * sign;
}

double GpsManager::ParseSpeedAngle(string *buff) {
	int floatPtIndex= buff->find('.');
	string leftSec= buff->substr(0, floatPtIndex);
	string rightSec= buff->substr(floatPtIndex+1, (buff->length()-2) - floatPtIndex);
	return stoi(leftSec, nullptr, 10) + (stoi(rightSec, nullptr, 10)/1000.0);
}

double GpsManager::ParseLatLong(string *buff) {
	const char *charBuff= buff->c_str();
	int floatPtIndex= buff->find('.');
	string latStr= buff->substr(0, floatPtIndex-2);
	double pos=(double)stoi(latStr, nullptr, 10);
	string minute= buff->substr(floatPtIndex-2, 2);
	pos += stoi(minute, nullptr, 10)/60.0;
	string rightSec= buff->substr(buff->length()-5, 3);
	string leftSec= buff->substr(floatPtIndex+1, (buff->length()-5) - (floatPtIndex+1));
	double sec= stoi(leftSec, nullptr, 10) + (stoi(rightSec, nullptr, 10)/1000.0);
	pos += sec / 3600.0;
	if (buff->c_str()[buff->length()-1] == 'S' || buff->c_str()[buff->length()-1] == 'W') {
		pos *= -1.0;
	}
	return pos;
}

HAL_StatusTypeDef GpsManager::ParseFrame() {
	string buff= "";
	int state= -1;
	for(int i=0; i < this->bufferIndex; i++) {
		buff += (char) this->buffer[i];
		//SOF FRAME Detection
		if (state < TIMESTEP) {
			//We only process GPRMC | Speed / Course & Positioning at same time
			//Under Sampling to ensure data consistency over time
			if (buff.find(POSE_ID) != string::npos && this->buffer[i] == ',') {
				state= TIMESTEP;
				buff = "";
			}
		}
		//FRAME Processing
		else {
			switch(state) {
				case TIMESTEP:
					if (this->buffer[i] == ',') {
						this->currentTime.tm_hour = stoi(buff.substr(0, 2), nullptr, 10);
						this->currentTime.tm_min = stoi(buff.substr(2, 2), nullptr, 10);
						this->currentTime.tm_sec = stoi(buff.substr(4, 2), nullptr, 10);
						buff = "";
						state= POSE_VALIDATION;
					}
					break;
				case POSE_VALIDATION:
					if (this->buffer[i] == ',') {
						const char *charBuff= buff.c_str();
						if(charBuff[0] == 'V') {
							state= INVALID;
						}
						if(charBuff[0] == 'A') {
							state= LATITUDE;
						}
						buff= "";
					}
					break;
				case LATITUDE:
					if (this->buffer[i-1] == ',' && buff.length()> 1) {
						this->latitude= ParseLatLong(&buff);
						state= LONGITUDE;
						buff= "";
						i++;
					}
					break;
				case LONGITUDE:
					if (this->buffer[i-1] == ',' && buff.length()> 1) {
						this->longitude= ParseLatLong(&buff);
						state= SPEED;
						i++;
						buff= "";
					}
					break;
				case SPEED:
					if (this->buffer[i] == ',') {
						if (buff.length() > 1 )
							this->speed= ParseSpeedAngle(&buff) * KNOT_MS;
						state= COURSE;
						buff= "";
					}
					break;
				case COURSE:
					if (this->buffer[i] == ',') {
						if (buff.length() > 1 )
							this->course= ParseSpeedAngle(&buff);
						state= DATETIME;
						buff= "";
					}
					break;
				case DATETIME:
					if (this->buffer[i] == ',') {
						this->currentTime.tm_mday = stoi(buff.substr(0, 2), nullptr, 10);
						this->currentTime.tm_mon = stoi(buff.substr(2, 2), nullptr, 10)-1;
						this->currentTime.tm_year = 2000 + stoi(buff.substr(4, 2), nullptr, 10) - 1900;
						state= MAGNETIC_VAR;
						buff= "";
					}
					break;
				case MAGNETIC_VAR:
					//DATA UNAVAILABLE
					if (this->buffer[i-1] == ',' && this->buffer[i] == ',') {
						i= this->bufferIndex;
					}
					//DATA AVAILABLE
					else if (this->buffer[i-1] == ',' && buff.length()> 1) {
						this->magneticVar= ParseMagneticVar(&buff);
					}
			}
		}
	}
}

void GpsManager::UpdateLocation(){
	//GET DATA Received by UART Communication
	if (this->incomingByte == this->sof) {
		this->bufferIndex = 0;
		this->checksum = 0x00;
		this->computeChecksum= true;
	}
	else if(this->bufferIndex >= 0 && this->incomingByte != '\n') {
		if (this->incomingByte == '*') {
			this->computeChecksum = false;
		}
		if(this->computeChecksum == true) {
			this->checksum ^= this->incomingByte;
		}
		this->buffer[bufferIndex]= this->incomingByte;
		bufferIndex += 1;
	}
	else if (this->incomingByte == '\n') {
		//COMPUTING CHECKSUM
		uint8_t receivedCheckSum= this->buffer[bufferIndex-3] << 4;
		if (this->buffer[bufferIndex-2] >= '0' && this->buffer[bufferIndex-2] <= '9') {
			int intByte= this->buffer[bufferIndex-2] - '0';
			receivedCheckSum += intByte;
		}
		else {
			receivedCheckSum += this->buffer[bufferIndex-2];
		}
		//VALID FRAME | We can parse frame for update data
		if (this->checksum == receivedCheckSum) {
			ParseFrame();
		}
		memset(this->buffer, 0, 128);
		bufferIndex= -1;
	}

	//Enable incomingByte Callback Reception
	HAL_UART_Receive_IT(this->sensorBus, &this->incomingByte, 1);
}

UART_HandleTypeDef *GpsManager::GetBus() {
	return this->sensorBus;
}

double GpsManager::GetLatitude(){
	return this->latitude;
}

double GpsManager::GetLongitude() {
	return this->longitude;
}

GpsManager::~GpsManager() {
	// TODO Auto-generated destructor stub
}

} /* namespace Osprai */
