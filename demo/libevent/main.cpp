#include <iostream>
// #include "Logger.h"
// #include "RedisClientInterface.h"


#include <event2/buffer.h>
#include <event2/bufferevent.h>
#include <event2/listener.h>

#include <arpa/inet.h>

#include <cerrno>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <csignal>

using namespace std;

void handle(int sig) {
    cout << "sig : " << sig <<endl;
}

static void echo_read_cb(struct bufferevent *bev, void *ctx) {
    cout << "echo_read_cb start" << endl;
    /* This callback is invoked when there is data to read on bev. */
    struct evbuffer *input  = bufferevent_get_input(bev);
    struct evbuffer *output = bufferevent_get_output(bev);

    /* Copy all the data from the input buffer to the output buffer. */
    evbuffer_add_buffer(output, input);

    shutdown(bufferevent_getfd(bev), SHUT_RDWR);
    
    // bufferevent_free(bev);
    cout << "echo_read_cb end" << endl;
}

static void echo_event_cb(struct bufferevent *bev, short events, void *ctx) {
    if(events & BEV_EVENT_ERROR) {
        // perror("Error from bufferevent");
        cout << "Error from bufferevent" << endl;
    }
    if(events & (BEV_EVENT_EOF)) {
        // printf("EOF\n");
        cout << "EOF" << endl;
        bufferevent_free(bev);
    }
    if(events & (BEV_EVENT_ERROR)) {
        cout << "ERROR %c\n" << evutil_socket_error_to_string(EVUTIL_SOCKET_ERROR()) << endl;
        // printf("ERROR %c\n", evutil_socket_error_to_string(EVUTIL_SOCKET_ERROR()));
        bufferevent_free(bev);
    }
}

static void
accept_conn_cb(struct evconnlistener *listener, evutil_socket_t fd, struct sockaddr *address, int socklen, void *ctx) {
    /* We got a new connection! Set up a bufferevent for it. */
    struct event_base  *base = evconnlistener_get_base(listener);
    struct bufferevent *bev  = bufferevent_socket_new(base, fd, BEV_OPT_CLOSE_ON_FREE);

    bufferevent_setcb(bev, echo_read_cb, NULL, echo_event_cb, NULL);

    bufferevent_enable(bev, EV_READ | EV_WRITE);
}

static void accept_error_cb(struct evconnlistener *listener, void *ctx) {
    struct event_base *base = evconnlistener_get_base(listener);
    int                err  = EVUTIL_SOCKET_ERROR();
    // fprintf(stderr,
    //         "Got an error %d (%s) on the listener. "
    //         "Shutting down.\n",
    //         err,
    //         evutil_socket_error_to_string(err));
    cout << "Got an error " << err << evutil_socket_error_to_string(err) << " on the listener. "
         << "Shutting down." << endl;

    event_base_loopexit(base, NULL);
}

int main(int argc, char **argv) {
    signal(SIGPIPE, handle);

    struct event_base     *base;
    struct evconnlistener *listener;
    struct sockaddr_in     sin {};

    int port = 4000;

    if(argc > 1) {
        port = atoi(argv[1]);
    }
    if(port <= 0 || port > 65535) {
        // puts("Invalid port");
        cout << "Invalid port" << endl;
        return 1;
    }

    base = event_base_new();
    if(!base) {
        // puts("Couldn't open event base");
        cout << "Couldn't open event base" << endl;
        return 1;
    }
    cout << "Couldn open event base" << endl;

    /* Clear the sockaddr before using it, in case there are extra
     * platform-specific fields that can mess us up. */
    memset(&sin, 0, sizeof(sin));
    /* This is an INET address */
    sin.sin_family = AF_INET;
    /* Listen on 0.0.0.0 */
    sin.sin_addr.s_addr = htonl(0);
    /* Listen on the given port. */
    sin.sin_port = htons(port);

    listener = evconnlistener_new_bind(base,
                                       accept_conn_cb,
                                       NULL,
                                       LEV_OPT_CLOSE_ON_FREE | LEV_OPT_REUSEABLE,
                                       -1,
                                       (struct sockaddr *)&sin,
                                       sizeof(sin));
    if(!listener) {
        // perror("Couldn't create listener");
        cout << "Couldn't create listener" << endl;
        return 1;
    }
    evconnlistener_set_error_cb(listener, accept_error_cb);

    event_base_dispatch(base);
    int err = EVUTIL_SOCKET_ERROR();
    // fprintf(stderr,
    //         "Got an error %d (%s) on the listener. "
    //         "Shutting down.\n",
    //         err,
    //         evutil_socket_error_to_string(err));
    cout << "Got an error " << err << evutil_socket_error_to_string(err) << " on the listener. "
         << "Shutting down." << endl;

    return 0;
}
