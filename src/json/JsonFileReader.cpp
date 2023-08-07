#include "wutils/experiment/json/JsonFileReader.h"

namespace wutils {

shared_ptr<JsonFileReader> JsonFileReader::FromFile(const std::string &file_name) {
    auto r = shared_ptr<JsonFileReader>();

    if(r->OpenFile(file_name)) {
        return r;
    }
    return nullptr;
}

bool JsonFileReader::OpenFile(const std::string &file_name) {
    std::ifstream io_file(file_name);
    if(!io_file.is_open()) {
        return false;
    }

    rapidjson::IStreamWrapper iSW(io_file);

    document_.ParseStream(iSW);
    if(document_.HasParseError()) {
        return false;
    }

    if(!document_.IsObject()) {
        return false;
    }

    return true;
}
auto JsonFileReader::GetData() { return document_.GetObject(); }

} // namespace wutils