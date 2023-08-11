#pragma once
#ifndef WUTILS_NET_DNS_HOSTSFILE_H
#define WUTILS_NET_DNS_HOSTSFILE_H

#include <fstream>
#include <map>

#include "wutils/base/HeadOnly.h"

#include "Error.h"
#include "Host.h"
#include "config/Definition.h"

namespace wutils::net::dns {

static std::string_view eraseSharp(std::string_view input) {
    auto i = input.find_first_of('#');
    if(i > input.size()) {
        return input;
    }

    return input.substr(0, i);
}

static void replaceToSpace(std::string &input) {
    for(auto &it : input) {
        if(it == '\t' || it == '\r' || it == '\n') {
            it = ' ';
        }
    }
}

static void spliteWithoutSpace(std::vector<std::string_view> &result, std::string_view input) {
    if(input.empty()) {
        return;
    }

    auto l = input.find_first_not_of(' ');
    auto r = input.find_first_of(' ', l);

    // all space ' '
    if(l > input.size()) {
        return;
    }
    //
    if(r > input.size()) {
        result.push_back(input.substr(l));
    }
    //
    else if(r < l) {
        spliteWithoutSpace(result, input.substr(l));
    }
    //
    else if(r > l) {
        result.push_back(input.substr(l, r - l));
        spliteWithoutSpace(result, input.substr(r));
    }
}

static void parseLineData(HostMap &map, std::string_view line_data) {
    if(line_data.empty()) {
        return;
    }

    std::vector<std::string_view> strings;
    spliteWithoutSpace(strings, eraseSharp(line_data));

    if(strings.empty()) {
        return;
    }
    if(strings.size() == 1) {
        throw std::system_error(eNetDNSError::HOSTS_FILE_PARSE_ERROR);
    }

    for(int i = 1; i < strings.size(); ++i)
        map.insert({std::string(strings[i]), std::string(strings[0])});
}

HEAD_ONLY HostMap ParseSystemHosts() noexcept(false) {
    std::fstream file(SYSTEM_HOSTS_FILE_PATH, std::ios::in);
    HostMap      result;

    if(!file.is_open()) {
        throw std::system_error(GetGenericError());
    }

    std::string line_data;
    while(std::getline(file, line_data)) {
        replaceToSpace(line_data);
        parseLineData(result, line_data);
    }

    return result;
}


} // namespace wutils::net::dns

#endif // WUTILS_NET_DNS_HOSTSFILE_H
