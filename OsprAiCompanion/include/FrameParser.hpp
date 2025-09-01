#ifndef FRAME_PARSER_HPP
#define FRAME_PARSER_HPP
#include "Utils.hpp"

#define SOF 0
#define FRAME_SIZE 1
#define DATA_ID 2
#define DATA_SIZE 3
#define DATA 4
#define CHECKSUM 5

class FrameParser {
    private:
		StaticVector<uint8_t, 10> Sof;
		StaticVector<StaticVector<uint8_t, 10>, 10> ParsingIds;
		StaticVector<StaticVector<char, 10>, 10> ParsingLabels;
	public:
		FrameParser() {}

		FrameParser(StaticVector<uint8_t, 10> sof,
						StaticVector<StaticVector<uint8_t, 10>, 10> parsingIds,
						StaticVector<StaticVector<char, 10>, 10> parsingLabels) {
			Sof= sof;
			ParsingIds= parsingIds;
			ParsingLabels= parsingLabels;
		}

        StaticVector<uint8_t, 10> getSOF() {
            return Sof;
        }

        StaticVector<StaticVector<char, 10>, 10> getLabels()  {
            return ParsingLabels;
        }

        StaticVector<StaticVector<float, 10>, 10> parseFrame(StaticVector<uint8_t, 500> frame) {
            StaticVector<StaticVector<float, 10>, 10> data;
            int parsingStep= FRAME_SIZE;
            uint8_t checksum= 0;
            int startIndex= findPattern(frame.data(), frame.size(), Sof.data(), Sof.size());
            if (startIndex >= 0) {
                int remainBytes= -1;
                int frameSize= 0;
                int dataSize= -1;
                int dataIdIndex=-1;
                bool isValidData= false;
                StaticVector<uint8_t, 100> buffer;
                for(int i= 0; i < Sof.size(); i++) {
                    checksum += Sof[i];
                }
                for (int i= startIndex+Sof.size(); i < frame.size(); i++) {
                    if (buffer.size() +1 == buffer.GetMaxSize()) {
                        break;
                    }
                    buffer.Add(frame[i]);
                    if (parsingStep != CHECKSUM)
                        checksum += frame[i];
                    if (parsingStep == FRAME_SIZE) {
                        frameSize= frame[i];
                        for(int j= 0; j < ParsingIds.size(); j++) {
                            data.Add(StaticVector<float, 10>());
                        }
                        remainBytes= frame[i]-Sof.size();
                        buffer.Clear();
                        parsingStep= DATA_ID;
                    }
                    else if (parsingStep == DATA_ID) {
                        for (int j= 0; j < ParsingIds.size(); j++) {
                            if (findPattern(buffer.data(), buffer.size(), ParsingIds[j].data(), ParsingIds[j].size()) >= 0) {
                                dataIdIndex= j;
                                buffer.Clear();
                                parsingStep= DATA_SIZE;
                                break;
                            }
                        }
                    } else if (parsingStep == DATA_SIZE) {
                        dataSize= frame[i];
                        parsingStep= DATA;
                        buffer.Clear();
                    } else if (parsingStep == DATA) {
                        if (data[dataIdIndex].size() > 3) {
                            int a= 1;
                        }
                        if (buffer.size() >= dataSize) {
                            //IF it's a measurement
                            if (dataSize >= sizeof(float)) {
                                for (int j= 0; j < buffer.size(); j += sizeof(float)) {
                                    vector<uint8_t> mesurement= buffer.SubVec(j, (int)(j+sizeof(float)));
                                    float floatVal;
                                    memcpy(&floatVal, mesurement.data(), sizeof(floatVal));
                                    data[dataIdIndex].Add(floatVal);
                                }
                            }
                            //ELSE it's a qualitative value (like value to Arm / DisArm the motors)
                            else {

                            }
                            if (remainBytes <=2)
                                parsingStep= CHECKSUM;
                            else
                                parsingStep= DATA_ID;
                            buffer.Clear();
                            dataSize= -1;
                            dataIdIndex= -1;
                        }
                    } else if (parsingStep == CHECKSUM) {
                        if (checksum != frame[i]) {
                            cout << "Received frame corrupted || Computed Checksum= " << (int)checksum << " Real Checksum= " << (int)frame[i] << endl;
                            uint8_t test= 0;
                            for (int k= 0; k < frameSize; k++) {
                                test += frame[startIndex + k];
                                cout << "Byte " << k << " = " << (int)frame[startIndex + k] << " | Checksum= " << (int)test << endl;
                            }
                            data.Clear();
                        }
                        break;
                    }
                    remainBytes--;
                }
            }
            if (parsingStep != CHECKSUM) {
                cout << "Received frame incomplete" << endl;
                data.Clear();
            }
            return data;
        }

        StaticVector<uint8_t, 500> encodeFrame(map<StaticVector<char, 10>, StaticVector<float, 10>> data) {
            StaticVector<uint8_t, 500> frame;
            int a= 1;
            /*string encodedFrame = Sof + "00";
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
            return encodedFrame;*/
            return frame;
        }
};

#endif