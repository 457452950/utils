#pragma once
#ifndef INCLUDE_WUTILS_NET_DNS_HOST_H
#define INCLUDE_WUTILS_NET_DNS_HOST_H

#include <map>
#include <string>

namespace wutils::net::dns {

using HostIp = std::string;

using HostName = std::string;

using HostMap = std::map<HostName, HostIp>;

} // namespace wutils::net::dns


#endif // INCLUDE_WUTILS_NET_DNS_HOST_H
