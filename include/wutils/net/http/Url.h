#pragma once
#ifndef WUTILS_NET_HTTP_URL_H
#define WUTILS_NET_HTTP_URL_H


#include <string>
#include <utility>
#include <map>
#include <vector>
#include <unordered_set>

#include "url/Scheme.h"

namespace wutils::net::http {


HEAD_ONLY const std::string_view URL_SCHEME_SIGN = "//:";

class Url {
public:
    Url() = default;
    explicit Url(std::string url) : url_(std::move(url)) { valid_ = parse(); }

    void Set(const std::string &url) {
        url_   = url;
        valid_ = parse();
    }
    std::string ToString() const { return url_; }
    std::string ToString() { return url_; }

    bool IsValid() { return valid_; }
    bool HasParameters() { return !parameters_.empty(); }

    std::string_view                                    GetPath() const { return path_; }
    std::string_view                                    GetScheme() const { return scheme_; }
    std::string_view                                    GetHost() const { return host_; }
    uint16_t                                            GetPort() const { return port_; }
    const std::map<std::string_view, std::string_view> &GetParameters() const { return parameters_; }
    std::string_view                                    GetAnchor() const { return anchor; }

private:
    bool parse() {
        try {
            std::string_view url = this->url_;

            auto sch_pos = url.find_first_of(URL_SCHEME_SIGN);
            if(sch_pos < url.size()) {
                // found //:
                this->scheme_ = url.substr(0, sch_pos);
                url           = url.substr(sch_pos + 3);

                // scheme not found.
                if(SchemeMap.find(this->scheme_) == SchemeMap.end()) {
                    return false;
                }
            }

            auto path_pos = url.find_first_of('/');
            auto port_pos = url.find_first_of(':');
            if(path_pos >= url.size() && port_pos >= url.size()) {
                // miss / and :
                this->host_ = url;
                return true;
            }
            if(path_pos >= url.size()) {
                // miss /
                this->host_ = url.substr(0, port_pos);
                auto port   = std::stoull(url_.substr(port_pos + 1));
                if(port > UINT16_MAX) {
                    return false;
                }
                this->port_ = port;
                return true;
            }
            if(path_pos < url.size() && port_pos < url.size()) {
                // find / and :
                this->host_ = url.substr(0, port_pos);
                auto port   = std::stoull(url_.substr(port_pos + 1, path_pos));
                if(port > UINT16_MAX) {
                    return false;
                }
                this->port_ = port;
            } else {
                // find /
                this->host_ = url.substr(0, path_pos);
            }

            url = url.substr(path_pos);

            auto q_pos = url.find_first_of('?');
            if(q_pos >= url.size()) {
                // miss ?
                this->path_ = url;
                return true;
            }

            this->path_ = url.substr(0, q_pos);

            url = url.substr(q_pos + 1);

            auto s_pos = url.find_first_of('#');
            if(s_pos >= url.size()) {
                // miss #
                return parseParameters(url) == 0;
            }

            auto res = parseParameters(url.substr(0, s_pos));
            if(res != 0) {
                return false;
            }

            this->anchor = url.substr(s_pos + 1);
            return true;
        } catch(std::invalid_argument e) {
            return false;
        }
    }; // namespace wutils::net::http

    // return 0 for success
    int parseParameters(std::string_view parameters) {
        if(parameters.empty()) {
            return 0;
        }
        std::vector<std::string_view> params;
        if(spliteParameters(params, parameters) != 0) {
            return 1;
        }
        for(auto it : params) {
            auto res = parseParameter(it);
            if(res != 0) {
                return 1;
            }
        }
        return 0;
    }

    // return 0 for success
    int spliteParameters(std::vector<std::string_view> &params, std::string_view input) {
        if(input.empty()) {
            return 0;
        }

        auto pos = input.find_first_of('&');
        if(pos >= input.size()) {
            params.push_back(input);
            return 0;
        }
        spliteParameters(params, input.substr(0, pos));
        spliteParameters(params, input.substr(pos + 1));
        return 0;
    }

    // return 0 for success
    int parseParameter(std::string_view pair) {
        auto pos = pair.find_first_of('=');
        if(pos == 0 || pos >= pair.size()) {
            return 1;
        }
        this->parameters_.insert({pair.substr(0, pos), pair.substr(pos + 1)});
        return 0;
    }

private:
    std::string url_;

    bool valid_{false};

    std::string_view                             scheme_{URL_SCHEME_HTTP_HEAD};
    std::string_view                             host_{};
    uint16_t                                     port_{0};
    std::string_view                             path_{};
    std::map<std::string_view, std::string_view> parameters_;
    std::string_view                             anchor{};
};

} // namespace wutils::net::http

#endif // WUTILS_NET_HTTP_URL_H
