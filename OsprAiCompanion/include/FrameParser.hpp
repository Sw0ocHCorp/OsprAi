#ifndef FRAME_PARSER_HPP
#define FRAME_PARSER_HPP
#include <Utils.hpp>

class FrameParser {
    private:
        string Sof;
        map<string, string> ParsingIds;
        map<string, string> EncodingIds;
    public:
        FrameParser(string sof, map<string, string> parsingIds, map<string, string> encodingIds) {
            Sof = sof;
            ParsingIds = parsingIds;
            EncodingIds = encodingIds;
        }

        virtual void parseFrame(string frame) = 0;
};

#endif