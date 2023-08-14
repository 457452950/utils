#pragma once
#ifndef UTILS_JSON_TOOLS_H
#define UTILS_JSON_TOOLS_H

#include <fstream>
#include <iostream>
#include <string>

#include "rapidjson/document.h"
#include "rapidjson/filereadstream.h"
#include "rapidjson/istreamwrapper.h"

#include "wutils/Error.h"
#include "wutils/base/HeadOnly.h"

namespace wutils {

HEAD_ONLY rapidjson::Document ParseFromFile(const std::string &file_name, Error &error) {
    rapidjson::Document document;

    std::ifstream io_file(file_name);
    if(!io_file.is_open()) {
        error = GetGenericError();
        return document;
    }

    rapidjson::IStreamWrapper iSW(io_file);

    document.ParseStream(iSW);

    return document;
}

} // namespace wutils

#endif // !UTILS_JSON_TOOLS_H
