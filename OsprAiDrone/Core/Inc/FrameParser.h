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
			StaticVector<uint8_t, 10> Sof;
			StaticVector<StaticVector<uint8_t, 10>, 10> ParsingIds;
			StaticVector<StaticVector<char, 10>, 10> ParsingLabels;
			/*string Sof;
			map<string, string> ParsingIds;*/
		public:
			FrameParser() {}

			FrameParser(StaticVector<uint8_t, 10> sof,
							StaticVector<StaticVector<uint8_t, 10>, 10> parsingIds,
							StaticVector<StaticVector<char, 10>, 10> parsingLabels) {
				Sof= sof;
				ParsingIds= parsingIds;
				ParsingLabels= parsingLabels;
			}

			StaticVector<StaticVector<float, 10>, 10> ParseFrame(StaticVector<uint8_t, 500> frame, bool isBigEndian= true) {
				StaticVector<StaticVector<float, 10>, 10> data;
				int parsingState= SOF;
				int dataId=-1;
				int dataSize= -1;
				uint8_t checksum= 0;
				int remainingBytes= -1;
				StaticVector<uint8_t, 100> buffer;
				for (int i= 0; i < frame.size(); i++) {
					if (buffer.size() == buffer.GetMaxSize())
						buffer.Clear();
					buffer.Add(frame[i]);
					if (parsingState != CHECKSUM) {
						checksum += frame[i];
					}
					if (parsingState == SOF) {
						if (FindPattern(buffer.data(), buffer.size(), Sof.data(), Sof.size()) >= 0) {
							buffer.Clear();
							parsingState= FRAME_SIZE;
							for (int j= 0; j < ParsingIds.size(); j++) {
								data.Add(StaticVector<float, 10>{});
							}
						}
					} else if(parsingState == FRAME_SIZE) {
						remainingBytes= frame[i] - Sof.size()-1;
						buffer.Clear();
						parsingState= DATA_ID;
					} else if(parsingState == DATA_ID) {
						for(int j= 0; j < ParsingIds.size(); j++) {
							if(FindPattern(buffer.data(), buffer.size(), ParsingIds[j].data(), ParsingIds[j].size()) >= 0) {
								dataId= j;
								parsingState= DATA_SIZE;
								buffer.Clear();
								break;
							}
						}
					} else if(parsingState == DATA_SIZE) {
						dataSize= frame[i];
						buffer.Clear();
						parsingState= DATA;
					} else if(parsingState == DATA) {
						if (buffer.size() >= dataSize) {
							for (int k= sizeof(float); k <= dataSize; k+= sizeof(float)) {
								float floatVal;
								memcpy(&floatVal, buffer.SubVec(k - sizeof(float), k, isBigEndian).data(), sizeof(float));
								data[dataId].Add(floatVal);
							}
							buffer.Clear();
							dataId= -1;
							dataSize= -1;
							if (remainingBytes <= 2) {
								parsingState= CHECKSUM;
							} else {
								parsingState= DATA_ID;
							}
						}
					} else {
						if(checksum != frame[i]) {
							data.Clear();
						}
						else {
							break;
						}
					}
					remainingBytes--;
				}
				if (parsingState != CHECKSUM)
					data.Clear();
				return data;
			}

			StaticVector<uint8_t, 500> EncodeFrame(StaticVector<StaticVector<uint8_t, 10>, 10>encodingIds, StaticVector<StaticVector<float, 10>, 10>data) {
				StaticVector<uint8_t, 500> frame;
				uint8_t checksum= 0;
				//HAL_GPIO_WritePin(Debug_GPIO_Port, Debug_Pin, GPIO_PIN_SET);
				//Add SOF to frame & compute SOF checksum
				for (int i= 0; i < Sof.size(); i++) {
					frame.Add(Sof[i]);
					checksum += Sof[i];
				}
				//HAL_GPIO_WritePin(Debug_GPIO_Port, Debug_Pin, GPIO_PIN_RESET);				//5.3us
				//Set a byte to O to fill it with frame size at the end of the encoding
				frame.Add(0x00);
				//Process all data to encode
				for (int i= 0; i < encodingIds.size(); i++) {
					//HAL_GPIO_WritePin(Debug_GPIO_Port, Debug_Pin, GPIO_PIN_SET);
					//Add Data ID to frame and compute his checksum
					for (int j= 0; j < encodingIds[i].size(); j++) {
						frame.Add(encodingIds[i][j]);
						checksum += encodingIds[i][j];
					}
					//HAL_GPIO_WritePin(Debug_GPIO_Port, Debug_Pin, GPIO_PIN_RESET);					// 8us
					//HAL_GPIO_WritePin(Debug_GPIO_Port, Debug_Pin, GPIO_PIN_SET);
					//Add data size to frame & compute his checksum
					frame.Add(data[i].size()*sizeof(float));
					checksum += (uint8_t)(data[i].size()  * sizeof(float));
					//HAL_GPIO_WritePin(Debug_GPIO_Port, Debug_Pin, GPIO_PIN_RESET);					// 3us

					//Add data to frame & compute his checksum
					//HAL_GPIO_WritePin(Debug_GPIO_Port, Debug_Pin, GPIO_PIN_SET);
					for(int j= 0; j < data[i].size(); j++) {

						uint8_t *floatVal= (uint8_t *)(&data[i][j]);
						for (int k= 0; k < 4; k++) {
							frame.Add(floatVal[k]);
							checksum += floatVal[k];
						}
					}
					//HAL_GPIO_WritePin(Debug_GPIO_Port, Debug_Pin, GPIO_PIN_RESET);					// 8us/float
				}
				checksum += (uint8_t)(frame.size()+1);
				frame.Add(checksum);
				frame[Sof.size()]= frame.size();
				return frame;
			}

			StaticVector<uint8_t, 10> GetSOF() {
				return Sof;
			}

			StaticVector<StaticVector<char, 10>, 10> GetParsingLabels() {
				return ParsingLabels;
			}
	};
}

#endif
