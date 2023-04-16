#pragma once
#ifndef UTILS_JSONFILEREADER_H
#define UTILS_JSONFILEREADER_H

#include <iostream>
#include <fstream>
#include <string>
#include <rapidjson/rapidjson.h>
#include <rapidjson/document.h>
#include <rapidjson/stringbuffer.h>
#include <rapidjson/filereadstream.h>
#include <rapidjson/istreamwrapper.h>
#include <rapidjson/prettywriter.h>

namespace wlb {

class JsonFileReader {
public:
    explicit JsonFileReader(const std::string &fileName);
    ~JsonFileReader();

    rapidjson::Value &operator[](const std::string &key) {
        return document_[key.c_str()];
    }
    rapidjson::Value &operator[](int key) {
        return document_[key];
    }

    bool HasMember(const std::string &key) {
        return document_.HasMember(key.c_str());
    }

    auto FindMember(const std::string &key) {
        return document_.FindMember(key.c_str());
    }
    auto MemberEnd() {
        return document_.MemberEnd();
    }

private:
    rapidjson::Document document_;
};

}

#endif // !UTILS_JSONFILEREADER_H




