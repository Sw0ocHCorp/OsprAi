#ifndef FRAME_PARSER_H
#define FRAME_PARSER_H

#define SOF 0
#define FRAME_SIZE 1
#define DATA_ID 2
#define DATA_SIZE 3
#define DATA 4
#define CHECKSM 5

#include "utils.h"

namespace Osprai {
	class FrameParser {
		private:
			string Sof;
			map<string, string> ParsingIds;
		public:
			FrameParser() {}

			FrameParser(string sof, map<string, string> parsingIds) {
				Sof = sof;
				ParsingIds = parsingIds;
			}

			map<string, vector<float>> parseFrame(string frame) {
				int currentStep = SOF;
				map<string, vector<float>> parsedData;
				string buffer;
				string dataId;
				int frameSize = 0;
				int dataSize = 0;
				unsigned char checksum = 0;
				for (int i = 0; i < frame.size(); i += 2) {
					frameSize--;
					buffer += frame[i];
					buffer += frame[i + 1];
					if (currentStep >FRAME_SIZE && frameSize <= 0)
						currentStep = CHECKSM;
					else
						checksum += stoi(frame.substr(i, 2), nullptr, 16);

					switch (currentStep) {
						case SOF:
							if (buffer == Sof) {
								currentStep = FRAME_SIZE;
								buffer.clear();
							}
							break;
						case FRAME_SIZE:
							frameSize = stoi(buffer, nullptr, 16) - ((this->Sof.size()/2)+1);
							currentStep = DATA_ID;
							buffer.clear();
							break;
						case DATA_ID:
							if (ParsingIds.find(buffer) != ParsingIds.end()) {
								parsedData[ParsingIds[buffer]] = vector<float>();
								dataId = ParsingIds[buffer];
								currentStep = DATA_SIZE;
								buffer.clear();
							}

							break;
						case DATA_SIZE:
							dataSize = stoi(buffer, nullptr, 16);
							currentStep = DATA;
							buffer.clear();
							break;
						case DATA:
							//Float Data => Measurements && Setpoints
							if ((buffer.size()/2) % 4 == 0 && dataSize % sizeof(float) == 0) {
								parsedData[dataId].push_back(hexStringToFloat(buffer));
								buffer.clear();
								if (parsedData[dataId].size() == dataSize / sizeof(float)) {
									currentStep = DATA_ID;
								}
							}
							//Int Data => Enums && Int Setpoints
							else if ((buffer.size()/2) % 2 == 0 && dataSize % 2 == 0 && dataSize % sizeof(float) != 0) {
								parsedData[dataId].push_back(static_cast<float>(stoi(buffer, nullptr, 16)));
								buffer.clear();
								if (parsedData[dataId].size() == dataSize / 2) {
									currentStep = DATA_ID;
								}
							}
							break;
						case CHECKSM:
							if (stoi(buffer, nullptr, 16) == checksum) {
								return parsedData;
							} else {
								cout << "Checksum error: expected " << static_cast<int>(checksum)
									 << ", got " << stoi(buffer, nullptr, 16) << endl;
							}
							break;
					}
				}
				parsedData.clear();
				return parsedData;
			}

			string encodeFrame(map<string, vector<float>> data) {
				string encodedFrame = Sof + "00";
				unsigned char checksum = 0;
				for (int i= 0; i < Sof.size(); i += 2) {
					checksum += stoi(Sof.substr(i, 2), nullptr, 16);
				}
				for (const auto& [id, values] : data) {
					for (int i= 0; i < id.size(); i += 2) {
						checksum += stoi(id.substr(i, 2), nullptr, 16);
					}
					encodedFrame += id;
					encodedFrame += uCharToHexString(values.size() * sizeof(float));
					checksum += values.size() * sizeof(float);
					for (const float& value : values) {
						string hexValue = floatToHexString(value);
						for (int i = 0; i < hexValue.size(); i += 2) {
							checksum += stoi(hexValue.substr(i, 2), nullptr, 16);
						}
						encodedFrame += hexValue;
					}
				}
				int frameSize = (encodedFrame.size() / 2)+1;
				checksum += (unsigned char)frameSize;
				encodedFrame += uCharToHexString(checksum);
				unsigned char msb= (unsigned char)frameSize >> 4;
				unsigned char lsb = (unsigned char)frameSize & 0x0F;
				encodedFrame[Sof.size()] = uCharToHexString(msb)[1];
				encodedFrame[Sof.size() + 1] = uCharToHexString(lsb)[1];
				return encodedFrame;
			}
	};
}

#endif
