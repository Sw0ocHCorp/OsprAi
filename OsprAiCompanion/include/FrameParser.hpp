#ifndef FRAME_PARSER_HPP
#define FRAME_PARSER_HPP
#include <Utils.hpp>

#define SOF 0
#define FRAME_SIZE 1
#define DATA_ID 2
#define DATA_SIZE 3
#define DATA 4
#define CHECKSUM 5

class FrameParser {
    private:
        string Sof;
        int CurrentStep = SOF;
        map<string, string> ParsingIds;
        map<string, string> EncodingIds;
    public:
        FrameParser(string sof, map<string, string> parsingIds, map<string, string> encodingIds) {
            Sof = sof;
            ParsingIds = parsingIds;
            EncodingIds = encodingIds;
        }

        map<string, vector<float>> parseFrame(string frame) {
            CurrentStep = SOF;
            map<string, vector<float>> parsedData;
            string buffer;
            string dataId;
            int frameSize = 0;
            int dataSize = 0;
            unsigned char checksum = 0;
            
            for (int i = 0; i < frame.length(); i += 2) {
                frameSize--;
                buffer += frame[i];
                buffer += frame[i + 1];
                if (CurrentStep >FRAME_SIZE && frameSize <= 0) 
                    CurrentStep = CHECKSUM;
                else 
                    checksum += stoi(frame.substr(i, 2), nullptr, 16);
                
                switch (CurrentStep) {
                    case SOF:
                        if (buffer == Sof) {
                            CurrentStep = FRAME_SIZE;
                            buffer.clear();
                        }
                        break;
                    case FRAME_SIZE:
                        frameSize += stoi(buffer, nullptr, 16);
                        CurrentStep = DATA_ID;
                        buffer.clear();
                        break;
                    case DATA_ID:
                        if (ParsingIds.find(buffer) != ParsingIds.end()) {
                            parsedData[ParsingIds[buffer]] = vector<float>();
                            dataId = ParsingIds[buffer];
                            CurrentStep = DATA_SIZE;
                            buffer.clear();
                        }
                        
                        break;
                    case DATA_SIZE:
                        dataSize = stoi(buffer, nullptr, 16);
                        CurrentStep = DATA;
                        buffer.clear();
                        break;
                    case DATA:
                        //Float Data => Measurements && Setpoints
                        if ((buffer.size()/2) % 4 == 0 && dataSize % sizeof(float) == 0) {
                            parsedData[dataId].push_back(hexStringToFloat(buffer));
                            buffer.clear();
                            if (parsedData[dataId].size() == dataSize / sizeof(float)) {
                                CurrentStep = DATA_ID;
                            }
                        }
                        //Int Data => Enums && Int Setpoints
                        else if ((buffer.size()/2) % 2 == 0 && dataSize % 2 == 0 && dataSize % sizeof(float) != 0) {
                            parsedData[dataId].push_back(static_cast<float>(stoi(buffer, nullptr, 16)));
                            buffer.clear();
                            if (parsedData[dataId].size() == dataSize / 2) {
                                CurrentStep = DATA_ID;
                            }
                        }
                        break;
                    case CHECKSUM:
                        if (stoi(buffer, nullptr, 16) == checksum) {
                            cout << "Frame parsed successfully" << endl;
                        } else {
                            cout << "Checksum error: expected " << static_cast<int>(checksum) 
                                 << ", got " << stoi(buffer, nullptr, 16) << endl;
                        }
                        break; 
                }
            }
            return parsedData;
        }

        string encodeFrame(map<string, vector<float>> data) {
            string encodedFrame = Sof + "00";
            unsigned char checksum = 0;
            for (int i= 0; i < Sof.size(); i += 2) {
                checksum += stoi(Sof.substr(i, 2), nullptr, 16);
            }
            for (const auto& [id, values] : data) {
                if (EncodingIds.find(id) != EncodingIds.end()) {
                    for (int i= 0; i < EncodingIds[id].size(); i += 2) {
                        checksum += stoi(EncodingIds[id].substr(i, 2), nullptr, 16);
                    }
                    encodedFrame += EncodingIds[id];
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

#endif