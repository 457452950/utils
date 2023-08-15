#pragma once
#ifndef UTIL_TEST_ECHO_MUL_SERVER_H
#define UTIL_TEST_ECHO_MUL_SERVER_H


#include <iostream>
#include <utility>
#include <signal.h>

#include "wutils/base/Defer.h"
#include "wutils/network/Network.h"

using namespace std;
using namespace wutils::network;

// #define CONTEXT event::EpollContext
#define CONTEXT event::SelectContext

/**
 * test_tcp_echo
 */
namespace test_echo_mul_server_config {

namespace srv {
namespace listen {
// constexpr char     *ip     = "0:0:0:0:0:0:0:0";
// constexpr int       port   = 4000;
// constexpr AF_FAMILY family = AF_FAMILY::INET6;
constexpr char     *ip     = "0.0.0.0";
constexpr int       port   = 12000;
constexpr AF_FAMILY family = AF_FAMILY::INET;
} // namespace listen
} // namespace srv


class Server {
public:
    static Server                                 *instance_;
    std::vector<std::shared_ptr<event::IOContext>> eps_;
    std::vector<std::thread *>                     threads;
    std::mutex                                     m;

    class TestSession : public Connection::Listener, public enable_shared_from_this<TestSession> {
    public:
        explicit TestSession(std::shared_ptr<Connection> ch_) : ch(std::move(ch_)) {
            ch->listener_ = this;
            ch->Init();
        }

        void OnDisconnect() override {
            std::cout << "disconnect" << std::endl;
            this->ch.reset();
            del();
        }
        void OnReceive(Data data) override {
            //        cout << "recv " << std::string((char *)data.data, (int)data.bytes) << " size " << data.bytes <<
            //        endl;
            ch->Send(data.data, data.bytes);
        }
        void OnError(wutils::Error error) override {
            std::cout << error << endl;
            this->ch.reset();
            del();
        }

        void del() {
            std::unique_lock<std::mutex> lock_1(instance_->m);
            instance_->sessions2.erase(shared_from_this());
        }

        // private:
        std::shared_ptr<Connection> ch;
    };

    class TestASession : public AConnection::Listener, public enable_shared_from_this<TestASession> {
    public:
        explicit TestASession(std::shared_ptr<AConnection> ch_) : ch(std::move(ch_)) {
            this->thread_id    = this_thread::get_id();
            ch->listener_      = this;
            buffer_.buffer     = new uint8_t[4096];
            buffer_.buffer_len = 4096;

            ch->AReceive(buffer_, AConnection::ARecvFlag::Keep);
        }
        ~TestASession() override {
            assert(isCurrent());
            delete[] buffer_.buffer;
        }

        void OnDisconnect() override {
            assert(isCurrent());
            ch->listener_ = nullptr;
            this->ch.reset();
            del();
        }
        void OnReceived(uint64_t len) override {
            //        cout << "recv " << std::string((char *)buffer.buffer, (int)buffer.buffer_len) << " size " <<
            //        buffer.buffer_len
            //             << endl;
            assert(isCurrent());
            ch->ASend({buffer_.buffer, len});
        }
        void OnSent(uint64_t len) override { assert(isCurrent()); }
        void OnError(wutils::Error error) override {
            assert(isCurrent());
            std::cout << error.message() << endl;
            ch->listener_ = nullptr;
            this->ch.reset();
            del();
        }
        void del() {
            std::unique_lock<std::mutex> lock_1(instance_->m);
            instance_->sessions.erase(shared_from_this());
        }
        bool isCurrent() { return this->thread_id == this_thread::get_id(); }

        // private:
        Buffer                  buffer_;
        shared_ptr<AConnection> ch;
        std::thread::id         thread_id;
    };

    std::function<void()> f;

    Server() {
        for(int i = 0; i < 6; ++i) {
            threads.push_back(new std::thread(&Server::server_thread, this));
        }

        //        f = [=]() {
        //            uint8_t n = random() % 4;
        //            LOG(LINFO, "random") << "random :" << (int)n;
        //            eps_[n]->Post([=]() { f(); });
        //        };
        //        std::this_thread::sleep_for(20ms);
        //        std::thread a(f);
        //        std::thread b(f);
        //        std::thread c(f);
        //        a.join();
        //        b.join();
        //        c.join();
    }
    ~Server() {
        this->sessions.clear();
        this->sessions2.clear();
        eps_.clear();
    }

    void join() {
        for(auto it : threads) {
            it->join();
        }
    }

    static void ac_cb(const NetAddress &local, const NetAddress &remote, shared_ptr<event::IOHandle> handler) {
        auto info = remote.Dump();

        //        LOG(LINFO, "accept2") << "accept : info " << std::get<0>(info) << " " << std::get<1>(info);
        auto ch = AConnection::Create(local, remote, std::move(handler));


        {
            std::unique_lock<std::mutex> lock_1(instance_->m);
            instance_->sessions.insert(make_shared<TestASession>(ch));
        }
    };
    static void ac_cb2(const NetAddress &local, const NetAddress &remote, shared_ptr<event::IOHandle> handler) {
        auto info = remote.Dump();

        LOG(LINFO, "accept2") << "accept : info " << std::get<0>(info) << " " << std::get<1>(info);
        auto ch = Connection::Create(local, remote, std::move(handler));
        {
            std::unique_lock<std::mutex> l(instance_->m);
            instance_->sessions2.insert(make_shared<TestSession>(ch));
        }
    };

    std::unordered_set<shared_ptr<TestASession>> sessions;

    std::unordered_set<shared_ptr<TestSession>> sessions2;

    static void err_cb(wutils::Error err) { LOG(LERROR, "server") << err.message(); }

    void server_thread() {
        using namespace srv;

        shared_ptr<event::IOContext> ep_ = CONTEXT::Create();
        ep_->Init();
        {
            std::unique_lock<std::mutex> lock_1(m);
            this->eps_.push_back(ep_);
        }

        NetAddress local_ed;
        if(!local_ed.Assign(listen::ip, listen::port, listen::family)) {
            return;
        }

        auto accp_channel      = Acceptor::Create(ep_);
        accp_channel->OnAccept = std::bind(ac_cb, local_ed, std::placeholders::_1, std::placeholders::_2);
        //        accp_channel->OnAccept = std::bind(ac_cb2, local_ed, std::placeholders::_1, std::placeholders::_2);
        accp_channel->OnError  = err_cb;

        if(!accp_channel->Open(local_ed.GetFamily())) {
            LOG(LERROR, "server") << wutils::GetGenericError().message();
            abort();
        }

        if(!accp_channel->SetPortReuse(true)) {
            LOG(LERROR, "server") << wutils::GetGenericError().message();
            abort();
        }

        if(accp_channel->Start(local_ed)) {
            LOG(LERROR, "server") << wutils::GetGenericError().message();
            abort();
        }

        auto info = accp_channel->GetLocal().Dump();
        LOG(LINFO, "server") << std::get<0>(info) << " " << std::get<1>(info);

        ep_->Loop();
        cout << wutils::GetGenericError().message() << endl;

        LOG(LINFO, "server") << "server thread end";
    }
};

Server *Server::instance_ = nullptr;


void handle_pipe(int signal) { LOG(LINFO, "signal") << "signal " << signal; }


} // namespace test_echo_mul_server_config


inline void test_echo_server() {
    using namespace test_echo_mul_server_config;
    cout << "-------------------- test tcp echo mul server --------------------" << endl;

    signal(SIGPIPE, handle_pipe); // 自定义处理函数
                                  //    signal(SIGINT, handle_pipe);  // 自定义处理函数

    Server::instance_ = new Server();

    Server::instance_->join();

    delete Server::instance_;

    cout << "-------------------- test tcp echo mul server end --------------------" << endl;
}


#endif // UTIL_TEST_ECHO_MUL_SERVER_H
