/*
 * GpsManager.h
 *
 *  Created on: Jan 18, 2025
 *      Author: nclsr
 */

#ifndef INC_GPSMANAGER_H_
#define INC_GPSMANAGER_H_

#include "main.h"
#include "SensorManager.h"
#include <string>
#include <ctime>

using namespace std;

#define KNOT_MS		0.51444444
#define DATA_LEN	256
#define POSE_ID "GPRMC"

#define TIMESTEP 		0
#define POSE_VALIDATION	1
#define LATITUDE		2
#define LONGITUDE   	3
#define SPEED			4
#define COURSE			5
#define DATETIME		6
#define MAGNETIC_VAR	7
#define CHECKSUM		8
#define INVALID			9

namespace Osprai {

class GpsManager : public SensorManager {
private:
	//Sensor Infos
	tm currentTime;
	double longitude;
	double latitude;
	double speed;
	double course;
	double magneticVar;
	//Var for Frame Parsing
	bool computeChecksum= false;
	uint8_t checksum;
	int bufferIndex= -1;
	uint8_t sof= '$';

	HAL_StatusTypeDef ParseFrame() {
		HAL_StatusTypeDef status= HAL_OK;
		string buff= "";
		int parsingState= -1;
		for(int i=0; i < (int)this->frameBuffer.size(); i++) {
			if (status != HAL_OK) {
				break;
			}
			buff += (char) this->frameBuffer[i];
			//SOF FRAME Detection
			if (parsingState < TIMESTEP) {
				//We only process GPRMC | Speed / Course & Positioning at same time
				//Under Sampling to ensure data consistency over time
				if (buff.find(POSE_ID) != string::npos && this->frameBuffer[i] == ',') {
					parsingState= TIMESTEP;
					buff = "";
				}
			}
			//FRAME Processing
			else {
				switch(parsingState) {
					case TIMESTEP:
						if (this->frameBuffer[i] == ',') {
							if (buff.length() > 1) {
								this->currentTime.tm_hour = stoi(buff.substr(0, 2), nullptr, 10);
								this->currentTime.tm_min = stoi(buff.substr(2, 2), nullptr, 10);
								this->currentTime.tm_sec = stoi(buff.substr(4, 2), nullptr, 10);
							}
							buff = "";
							parsingState= POSE_VALIDATION;
						}
						break;
					case POSE_VALIDATION:
						if (this->frameBuffer[i] == ',') {
							const char *charBuff= buff.c_str();
							if(charBuff[0] == 'V') {
								parsingState= INVALID;
								status= HAL_ERROR;
							}
							if(charBuff[0] == 'A') {
								parsingState= LATITUDE;
							}
							buff= "";
						}
						break;
					case LATITUDE:
						if (this->frameBuffer[i-1] == ',' && buff.length()> 1) {
							this->latitude= ParseLatLong(&buff);
							parsingState= LONGITUDE;
							buff= "";
							i++;
						}
						break;
					case LONGITUDE:
						if (this->frameBuffer[i-1] == ',' && buff.length()> 1) {
							this->longitude= ParseLatLong(&buff);
							parsingState= SPEED;
							i++;
							buff= "";
						}
						break;
					case SPEED:
						if (this->frameBuffer[i] == ',') {
							if (buff.length() > 1 )
								this->speed= ParseSpeedAngle(&buff) * KNOT_MS;
							parsingState= COURSE;
							buff= "";
						}
						break;
					case COURSE:
						if (this->frameBuffer[i] == ',') {
							if (buff.length() > 1 )
								this->course= ParseSpeedAngle(&buff);
							parsingState= DATETIME;
							buff= "";
						}
						break;
					case DATETIME:
						if (this->frameBuffer[i] == ',') {
							this->currentTime.tm_mday = stoi(buff.substr(0, 2), nullptr, 10);
							this->currentTime.tm_mon = stoi(buff.substr(2, 2), nullptr, 10)-1;
							this->currentTime.tm_year = 2000 + stoi(buff.substr(4, 2), nullptr, 10) - 1900;
							parsingState= MAGNETIC_VAR;
							buff= "";
						}
						break;
					case MAGNETIC_VAR:
						//DATA UNAVAILABLE
						if (this->frameBuffer[i-1] == ',' && this->frameBuffer[i] == ',') {
							i= this->frameBuffer.size();
						}
						//DATA AVAILABLE
						else if (this->frameBuffer[i-1] == ',' && buff.length()> 1) {
							this->magneticVar= ParseMagneticVar(&buff);
						}
				}
			}
		}
		this->frameBuffer.clear();
		return status;
	}

	double ParseLatLong(string *buff) {
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

	double ParseSpeedAngle(string *buff) {
		int floatPtIndex= buff->find('.');
		string leftSec= buff->substr(0, floatPtIndex);
		string rightSec= buff->substr(floatPtIndex+1, (buff->length()-2) - floatPtIndex);
		return stoi(leftSec, nullptr, 10) + (stoi(rightSec, nullptr, 10)/1000.0);
	}

	double ParseMagneticVar(string *buff) {
		int floatPtIndex= buff->find('.');
		string leftSec= buff->substr(0, floatPtIndex);
		string rightSec= buff->substr(floatPtIndex+1, (buff->length()-3) - floatPtIndex);
		double sign= 1.0;
		if(buff->c_str()[buff->length()-1] == 'W')
			sign= -1.0;
		return stoi(leftSec, nullptr, 10) + (stoi(rightSec, nullptr, 10)/1000.0) * sign;
	}

	uint8_t ComputeNMEACheckSum(vector<uint8_t> frame) {
		int frameCheckSumIndex= FindPattern(frame, vector<uint8_t> {'*'});
		uint8_t checkSum= 0;
		for (int i= 0; i < frameCheckSumIndex; i++) {
			checkSum ^= frame.at(i);
		}
		return checkSum;
	}

	HAL_StatusTypeDef AnswerToRequest(vector<uint8_t> request) {
		return HAL_OK;
	}

	HAL_StatusTypeDef ExtractData(bool enableInterrupt) {
		//If Blocking Mode, we get incomingByte to process the new data
		if (enableInterrupt == false) {
			HAL_StatusTypeDef status= HAL_UART_Receive(getUARTInterface(), &this->incomingByte, 1, 10);
			if (status != HAL_OK)
				return status;
		}
		//GET DATA Received by UART Communication
		this->frameBuffer.push_back(this->incomingByte);
		//If END OF FRAME Detected, process frameBuffer
		if (this->incomingByte == '\n') {
			//SOF Detection
			int startingIndex= FindPattern(this->frameBuffer, getRawSof());
			//If SOF Detected, delete all bytes before SOF
			if (startingIndex>= 0) {
				this->frameBuffer.erase(this->frameBuffer.begin(), this->frameBuffer.begin() + startingIndex + getRawSof().size());
				int frameCheckSumIndex= FindPattern(this->frameBuffer, vector<uint8_t> {'*'});
				if (((ComputeNMEACheckSum(this->frameBuffer) == CharToByte(this->frameBuffer.at(frameCheckSumIndex+1)) << 4) | (CharToByte(this->frameBuffer.at(frameCheckSumIndex+2))) << 0) &&
						FindPattern(this->frameBuffer, vector<uint8_t> {'R', 'M', 'C'})>= 0) {
					//If Interrupt Mode, call new Interrupt on data reception to maintain the loop
					if (enableInterrupt) {
						HAL_UART_Receive_IT(this->getUARTInterface(), &this->incomingByte, 1);
					}
					return ParseFrame();
				}
				else {
					this->frameBuffer.clear();
					//If Interrupt Mode, call new Interrupt on data reception to maintain the loop
					if (enableInterrupt) {
						HAL_UART_Receive_IT(this->getUARTInterface(), &this->incomingByte, 1);
					}
					return HAL_ERROR;
				}
			}
		}
		// If we are in Interrupt Mode, and we don't detect END OF FRAME yet | Call new Interrupt on data reception to maintain the loop
		if (enableInterrupt) {
			HAL_UART_Receive_IT(this->getUARTInterface(), &this->incomingByte, 1);
		}
		return HAL_BUSY;
	}

public:
	uint8_t incomingByte= 0;
	uint8_t *buffer;
	vector<uint8_t> frameBuffer;
	GpsManager(int bufferSize) : SensorManager(bufferSize) {

	}

	virtual ~GpsManager() {

	}

	HAL_StatusTypeDef SensorConfiguration(map<uint8_t, vector<uint8_t>> *configuration= nullptr) {
		return HAL_OK;
	}
};

} /* namespace Osprai */

#endif /* INC_GPSMANAGER_H_ */
