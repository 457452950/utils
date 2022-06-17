#include "../../include/JsonFileReader.h"

namespace wlb {

JsonFileReader::JsonFileReader(const std::string &fileName) {
    std::ifstream ifile(fileName);
    if (!ifile.is_open()) {
        exit(-1);
    }
    rapidjson::IStreamWrapper iSW(ifile);

    document_.ParseStream(iSW);
    if (document_.HasParseError()) {
    }

    if (!document_.IsObject()) {
    }

}

JsonFileReader::~JsonFileReader() = default;

}