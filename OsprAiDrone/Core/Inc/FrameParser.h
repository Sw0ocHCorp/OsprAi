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
					checksum += data[i].size()  * sizeof(float);
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
				frame.Add(checksum);
				frame[Sof.size()]= frame.size();
				return frame;
			}
	};
}

#endif
