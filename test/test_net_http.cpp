#include <gtest/gtest.h>

#include "wutils/net/http/base/Tools.h"
#include "wutils/net/http/Url.h"
#include "wutils/net/http/url/Scheme.h"


TEST(net, http_get_date) {
    using namespace wutils::net;
    GTEST_LOG_(INFO) << http::GetFormatDate();
}

TEST(net, http_parse_url) {
    using namespace wutils::net;

    {
        http::Url url1("www.baidu.com");
        ASSERT_TRUE(url1.IsValid());
        ASSERT_EQ(url1.ToString(), "www.baidu.com");
        ASSERT_EQ(url1.GetPath(), "");
        ASSERT_EQ(url1.GetAnchor(), "");
        ASSERT_EQ(url1.GetHost(), std::string_view("www.baidu.com"));
        ASSERT_EQ(url1.GetPort(), 0);
        ASSERT_EQ(url1.GetScheme(), "http");
        ASSERT_TRUE(url1.GetParameters().empty());
    }
    {
        http::Url url1("http://www.baidu.com");
        ASSERT_TRUE(url1.IsValid());
        ASSERT_EQ(url1.GetPath(), "");
        ASSERT_EQ(url1.GetAnchor(), "");
        ASSERT_EQ(url1.GetHost(), std::string_view("www.baidu.com"));
        ASSERT_EQ(url1.GetPort(), 0);
        ASSERT_EQ(url1.GetScheme(), "http");
        ASSERT_TRUE(url1.GetParameters().empty());
    }
    {
        http::Url url1("https://mbd.baidu.com/newspage/data/"
                       "landingsuper?context=%7B%22nid%22%3A%22news_9397401079372333198%22%7D&n_type=-1&p_from=-1");
        ASSERT_TRUE(url1.IsValid());
        ASSERT_EQ(url1.GetPath(), std::string_view("/newspage/data/landingsuper"));
        ASSERT_EQ(url1.GetAnchor(), "");
        ASSERT_EQ(url1.GetHost(), std::string_view("mbd.baidu.com"));
        ASSERT_EQ(url1.GetPort(), 0);
        ASSERT_EQ(url1.GetScheme(), "https");
        ASSERT_EQ(url1.GetParameters().size(), 3);
        for(auto it : url1.GetParameters()) {
            GTEST_LOG_(INFO) << it.first << " = " << it.second;
        }
    }
    {
        http::Url url1("https://www.python100.com/html/5Z49TC2NCA98.html");
        ASSERT_TRUE(url1.IsValid());
        ASSERT_EQ(url1.GetPath(), std::string_view("/html/5Z49TC2NCA98.html"));
        ASSERT_EQ(url1.GetAnchor(), "");
        ASSERT_EQ(url1.GetHost(), std::string_view("www.python100.com"));
        ASSERT_EQ(url1.GetPort(), 0);
        ASSERT_EQ(url1.GetScheme(), std::string_view("https"));
        ASSERT_TRUE(url1.GetParameters().empty());
    }
    {
        http::Url url1("https://www.baidu.com/"
                       "s?wd=%E5%B8%A6%E6%9C%89%20%23%20%E7%9A%84url&rsv_spt=1&rsv_iqid=0xa2ec01cf0004f6a7&issp=1&f=8&"
                       "rsv_bp=1&rsv_idx=2&ie=utf-8&rqlang=cn&tn=baiduhome_pg&rsv_enter=1&rsv_dl=tb&oq=%25E5%25B8%25A6%"
                       "25E6%259C%2589%2520%2523%2520%25E7%259A%2584%25E7%25BD%2591%25E9%25A1%25B5&rsv_btype=t&inputT="
                       "839&rsv_t=9b17W0Hm7BS9nNTySVX2H8yXlmsgK9eXxH8l9sZ04jvcx6E3UCiMVpc9JY7Lvd0OVOE7&rsv_pq="
                       "e1298e1a000a2ee0&rsv_sug3=41&rsv_sug1=40&rsv_sug7=100&rsv_sug2=0&rsv_sug4=1681");
        ASSERT_TRUE(url1.IsValid());
        ASSERT_EQ(url1.GetPath(), std::string_view("/s"));
        ASSERT_EQ(url1.GetAnchor(), "");
        ASSERT_EQ(url1.GetHost(), std::string_view("www.baidu.com"));
        ASSERT_EQ(url1.GetPort(), 0);
        ASSERT_EQ(url1.GetScheme(), "https");
        ASSERT_EQ(url1.GetParameters().size(), 22);
        for(auto it : url1.GetParameters()) {
            GTEST_LOG_(INFO) << it.first << " = " << it.second;
        }
    }
    {
        http::Url url1("htttp://www.baidu.com");
        ASSERT_FALSE(url1.IsValid());
    }
    {
        http::Url url1("http://www.baidu.com::79");
        ASSERT_FALSE(url1.IsValid());
    }
    {
        http::Url url1("http://www.baidu.com:65536");
        ASSERT_FALSE(url1.IsValid());
    }
}

TEST(net, url_scheme) {
    using namespace wutils::net::http;
    {
        auto r = ServiceHelper::GetServiceDefaultPort("http");
        ASSERT_TRUE(r.valid());
        auto res = r.get();
        ASSERT_TRUE(res.Success()) << res.GetError().message();
        ASSERT_EQ(res.Get(), 80);
    }
    {
        auto r = ServiceHelper::GetServiceDefaultPort("https");
        ASSERT_TRUE(r.valid());
        auto res = r.get();
        ASSERT_TRUE(res.Success()) << res.GetError().message();
        ASSERT_EQ(res.Get(), 443);
    }
    {
        auto r = ServiceHelper::GetServiceDefaultPort("ssh");
        ASSERT_TRUE(r.valid());
        auto res = r.get();
        ASSERT_TRUE(res.Success()) << res.GetError().message();
        ASSERT_EQ(res.Get(), 22);
    }
    {
        auto r = ServiceHelper::GetServiceDefaultPort("ws");
        ASSERT_TRUE(r.valid());
        auto res = r.get();
        ASSERT_FALSE(res.Success()) << res.Get() << res.GetError().message();
    }
}
