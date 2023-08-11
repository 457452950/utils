#include <gtest/gtest.h>

#include "wutils/net/dns/HostsFile.h"


TEST(ssl, parse) {
    using namespace wutils::net;
    auto map = dns::ParseSystemHosts();
    for(const auto &it : map) {
        GTEST_LOG_(INFO) << "host name : " << it.first << "  host ip : " << it.second;
    }
    GTEST_LOG_(INFO) << "map size " << map.size();
}
