#pragma once
#ifndef WUTILS_NET_HTTP_BASE_TOOLS_H
#define WUTILS_NET_HTTP_BASE_TOOLS_H

#include <string>

#include <fmt/format.h>
#include <fmt/chrono.h>

namespace wutils::net::http {

// Date: Wed, 21 Oct 2015 07:28:00 GMT
// Date: <day-name>, <day> <month> <year> <hour>:<minute>:<second> GMT
std::string GetFormatDate() {
    std::time_t t = std::time(nullptr);
    std::tm     now_tm{};
    gmtime_r(&t, &now_tm);

    return fmt::format("{:%a, %d %b %Y %T} GMT", now_tm);
}


} // namespace wutils::net::http

#endif // WUTILS_NET_HTTP_BASE_TOOLS_H
