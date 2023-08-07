#pragma once
#ifndef UTILS_JSONFILEREADER_H
#define UTILS_JSONFILEREADER_H

#include <fstream>
#include <iostream>
#include <string>

#include <rapidjson/document.h>
#include <rapidjson/filereadstream.h>
#include <rapidjson/istreamwrapper.h>
#include <rapidjson/prettywriter.h>
#include <rapidjson/rapidjson.h>
#include <rapidjson/stringbuffer.h>

#include "wutils/SharedPtr.h"

namespace wutils {

class JsonFileReader {
public:
    shared_ptr<JsonFileReader> FromFile(const std::string &file_name);
    ~JsonFileReader() = default;

private:
    JsonFileReader() = default;
    bool OpenFile(const std::string &file_name);

public:
    auto GetData();

private:
    rapidjson::Document document_;
};

} // namespace wutils

#endif // !UTILS_JSONFILEREADER_H
