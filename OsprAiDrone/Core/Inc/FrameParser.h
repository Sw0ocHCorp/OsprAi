#ifndef FRAME_PARSER_H
#define FRAME_PARSER_H

#define SOF 0
#define FRAME_SIZE 1
#define DATA_ID 2
#define DATA_SIZE 3
#define DATA 4
#define CHECKSM 5

#include "utils.h"

namespace OsprAi {
	class FrameParser {
		private:
			StaticVector<char, 10> Sof;
			StaticVector<StaticVector<char, 10>, 10> ParsingIds;
			StaticVector<StaticVector<char, 10>, 10> ParsingLabels;
			/*string Sof;
			map<string, string> ParsingIds;*/
		public:
			FrameParser() {}

			FrameParser(StaticVector<char, 10> sof,
							StaticVector<StaticVector<char, 10>, 10> parsingIds,
							StaticVector<StaticVector<char, 10>, 10> parsingLabels) {
				Sof= sof;
				ParsingIds= parsingIds;
				ParsingLabels= parsingLabels;
			}

			StaticVector<char, 100> EncodeFrame(StaticVector<StaticVector<char, 10>, 10>encodingIds, StaticVector<StaticVector<float, 10>, 10>data) {
				StaticVector<char, 100> frame;
				uint8_t checksum= 0;
				//Add SOF to frame & compute SOF checksum
				frame.Add((char *)Sof.GetData(), Sof.GetSize());
				for (int i= 0; i < (int)Sof.GetSize(); i+=2) {
					string byte;
					byte += Sof[i];
					byte += Sof[i+1];
					checksum += stoi(byte.c_str(), nullptr, 16);
				}
				//Set a byte to O to fill it with frame size at the end of the encoding
				frame.Add('0');
				frame.Add('0');
				//Process all data to encode
				for (int i= 0; i < encodingIds.GetSize(); i++) {
					//Add Data ID to frame and compute his checksum
					frame.Add((char *)encodingIds[i].GetData(), encodingIds[i].GetSize());
					string byte;
					for (int j= 0; j < encodingIds[i].GetSize(); j++) {
						byte += encodingIds[i][j];
						byte += encodingIds[i][j+1];
						checksum += stoi(byte.c_str(), nullptr, 16);
					}
					//Add data size to frame & compute his checksum
					frame.Add((char *)(uCharToHexString(data[i].GetSize() * sizeof(float)).c_str()), 2);
					checksum += data[i].GetSize()  * sizeof(float);
					//Add data to frame & compute his checksum
					for(int j= 0; j < data[i].GetSize(); j++) {
						string floatStr= floatToHexString(data[i][j]);
						frame.Add((char *)(floatStr.c_str()), floatStr.size());
						for (int k= 0; k < (int)floatStr.size(); k+=2) {
							checksum += stoi(floatStr.substr(k, 2), nullptr, 16);
						}
					}
				}


				return frame;
			}

			/*
			string EncodeFrame(map<string, vector<float>> data) {
				string encodedFrame = Sof + "00";
				uint8_t *test= (uint8_t *)encodedFrame.data();
				unsigned char checksum = 0;*/
				/*for (int i= 0; i < (int)Sof.size(); i += 2) {
					checksum += stoi(Sof.substr(i, 2), nullptr, 16);
				}*/
				/*for (const auto& [id, values] : data) {
					for (int i= 0; i < (int)id.size(); i += 2) {
						checksum += stoi(id.substr(i, 2), nullptr, 16);
					}
					encodedFrame += id;
					encodedFrame += uCharToHexString(values.size() * sizeof(float));
					test= (uint8_t *)encodedFrame.data();
					checksum += values.size() * sizeof(float);
					for (const float& value : values) {
						string hexValue = floatToHexString(value);
						for (int i = 0; i < (int)hexValue.size(); i += 2) {
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
			}*/

			/*FrameParser(string sof, map<string, string> parsingIds) {
				Sof = sof;
				ParsingIds = parsingIds;
			}

			map<string, vector<float>> ParseFrame(string frame) {
				int currentStep = SOF;
				map<string, vector<float>> parsedData;
				string buffer;
				string dataId;
				int frameSize = 0;
				int dataSize = 0;
				unsigned char checksum = 0;
				for (int i = 0; i < (int)frame.size(); i += 2) {
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
								if ((int)parsedData[dataId].size() == dataSize / 2) {
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

			string EncodeFrame(map<string, vector<float>> data) {
				string encodedFrame = Sof + "00";
				uint8_t *test= (uint8_t *)encodedFrame.data();
				unsigned char checksum = 0;*/
				/*for (int i= 0; i < (int)Sof.size(); i += 2) {
					checksum += stoi(Sof.substr(i, 2), nullptr, 16);
				}*/
				/*for (const auto& [id, values] : data) {
					for (int i= 0; i < (int)id.size(); i += 2) {
						checksum += stoi(id.substr(i, 2), nullptr, 16);
					}
					encodedFrame += id;
					encodedFrame += uCharToHexString(values.size() * sizeof(float));
					test= (uint8_t *)encodedFrame.data();
					checksum += values.size() * sizeof(float);
					for (const float& value : values) {
						string hexValue = floatToHexString(value);
						for (int i = 0; i < (int)hexValue.size(); i += 2) {
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
			}*/
	};
}

#endif
