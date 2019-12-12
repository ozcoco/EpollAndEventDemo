//
// Created by ozcomcn on 12/13/19.
//

#pragma once

#include <iostream>
#include "ThreadPool.h"
#include "rxcpp/rx.hpp"
#include "hello-world.h"

namespace Rx
{
    using namespace rxcpp;
    using namespace rxcpp::sources;
    using namespace rxcpp::operators;
    using namespace rxcpp::util;
}

using namespace Rx;

#include <regex>
#include <random>

using namespace std;
using namespace std::chrono;


void test();

void test2();


void testEventBut()
{
    // create thread pool with 4 worker threads
    ThreadPool pool(4);
    // enqueue and store future
    auto result = pool.enqueue([](int answer) { return answer; }, 42);
    // get result from future
    std::cout << result.get() << std::endl;


}


#include "eventbus/EventBus.h"
#include "eventbus/EventCollector.h"
#include "eventbus/AsyncEventBus.h"

Dexode::EventBus bus{};
//Dexode::AsyncEventBus asyncBus{};
Dexode::EventCollector collector{&bus};


void unlisten()
{
    collector.unlistenAll();
}

namespace Events // optional namespace
{
    struct Gold
    {
        int goldReceived = 0;
    };

    struct OK
    {
    };


    struct MyMessage
    {
        std::string msg;
    };

}

void testEventBut2()
{
    const auto &token = bus.listen<Events::Gold>
            ([](const auto &event) // listen with lambda
             {
                 std::cout << "I received gold: " << event.goldReceived << "!" << std::endl;
             });

    bus.notify(Events::Gold{12});

    bus.unlistenAll(token);

    collector.listen<Events::Gold>([](const Events::Gold &event) // register listener
                                   {

                                       std::cout << "I received gold: " << event.goldReceived << "!" << std::endl;

                                   });

    bus.notify(Events::Gold{20});


    collector.unlistenAll();


    collector.listen<Events::MyMessage>([](const Events::MyMessage &event) // register listener
                                        {

                                            std::cout << "------> Upper: " << event.msg << std::endl;

                                        });

}


void testRXCpp()
{
    random_device rd;   // non-deterministic generator
    mt19937 gen(rd());
    uniform_int_distribution<> dist(4, 18);

    // for testing purposes, produce byte stream that from lines of text
    auto bytes = range(0, 10) |
                 flat_map([&](int i) {
                     auto body = from((uint8_t) ('A' + i)) |
                                 repeat(dist(gen)) |
                                 as_dynamic();
                     auto delim = from((uint8_t) '\r');
                     return from(body, delim) | concat();
                 }) |
                 window(17) |
                 flat_map([](observable<uint8_t> w) {
                     return w |
                            reduce(
                                    vector<uint8_t>(),
                                    [](vector<uint8_t> v, uint8_t b) {
                                        v.push_back(b);
                                        return v;
                                    }) |
                            as_dynamic();
                 }) |
                 tap([](vector<uint8_t> &v) {
                     // print input packet of bytes
                     copy(v.begin(), v.end(), ostream_iterator<long>(cout, " "));
                     cout << endl;
                 });

    //
    // recover lines of text from byte stream
    //

    auto removespaces = [](string s) {
        s.erase(remove_if(s.begin(), s.end(), ::isspace), s.end());
        return s;
    };

    // create strings split on \r
    auto strings = bytes |
                   concat_map([](vector<uint8_t> v) {
                       string s(v.begin(), v.end());
                       regex delim(R"/(\r)/");
                       cregex_token_iterator cursor(&s[0], &s[0] + s.size(), delim, {-1, 0});
                       cregex_token_iterator end;
                       vector<string> splits(cursor, end);
                       return iterate(move(splits));
                   }) |
                   filter([](const string &s) {
                       return !s.empty();
                   }) |
                   publish() |
                   ref_count();

    // filter to last string in each line
    auto closes = strings |
                  filter(
                          [](const string &s) {
                              return s.back() == '\r';
                          }) |
                  Rx::map([](const string &) { return 0; });

    // group strings by line
    auto linewindows = strings |
                       window_toggle(closes | start_with(0), [=](int) { return closes; });

    // reduce the strings for a line into one string
    auto lines = linewindows |
                 flat_map([&](observable<string> w) {
                     return w | start_with<string>("") | sum() | Rx::map(removespaces);
                 });

    // print result
    lines |
    subscribe<string>(println(cout));
}


void testLibevent()
{
    main2();
}


#include <sys/types.h>
#include <unistd.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <sys/stat.h>
#include <sys/un.h>
#include <error.h>
#include <sys/epoll.h>
#include <cstring>
#include <algorithm>
#include <functional>
#include <utility>
#include <fcntl.h>

template<typename __Type>
class ToUpper
{
public:
    inline __Type operator()(__Type &__t)
    {
        return toupper(__t);
    }
};

#define PROTOCOL 0
#define PORT 9999
#define DOMAIN AF_INET
#define TYPE SOCK_STREAM
#define S_IP "127.0.0.1"

void test()
{

    int listen_fd, c_fd;

    struct sockaddr_in sockAddr
            {
            }, acceptAddr{};

    socklen_t c_addr_len;

    char buf[BUFSIZ];

    memset(buf, 0, BUFSIZ);

    memset(&sockAddr, 0, sizeof(sockAddr));

    memset(&acceptAddr, 0, sizeof(acceptAddr));

    listen_fd = socket(DOMAIN, TYPE, PROTOCOL);

    if (listen_fd < 0)
    {
        perror("create socket failure !");

        exit(errno);
    }

    sockAddr.sin_family = AF_INET;

    sockAddr.sin_port = htons(PORT);

    sockAddr.sin_addr.s_addr = htonl(INADDR_ANY);

    if (bind(listen_fd, (struct sockaddr *) &sockAddr, sizeof(sockAddr)) != 0)
    {
        perror("bind socket failure !");

        exit(errno);
    }

    if (listen(listen_fd, 200) != 0)
    {
        perror("listen socket failure !");

        exit(errno);
    }

    c_fd = accept(listen_fd, (struct sockaddr *) &acceptAddr, &c_addr_len);

    if (c_fd < 0)
    {
        perror("accept socket failure !");

        exit(errno);
    }

    while (true)
    {
        size_t len = read(c_fd, buf, sizeof(buf));

        if (len == 0)
            continue;

        if (strstr(buf, "exit"))
        {
            break;
        }

        printf("%s\n", buf);

        std::transform(buf, buf + len, buf, ToUpper<char>());

        write(c_fd, buf, len);

        memset(buf, 0, len);
    }

    close(c_fd);

    close(listen_fd);

    printf("exit!!!");
}

/**
 * set fd nonblock
 *
 * **/
void set_nonblock(int fd)
{
    fcntl(fd, F_SETFL, O_NONBLOCK);
}

/**
 * set sock resume
 *
 * **/
void set_sock_reuse(int sock_fd)
{

    int reuse = 1;

    if (setsockopt(sock_fd, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT, &reuse, sizeof(reuse)) == -1)
        // if (setsockopt(sock_fd, SOL_SOCKET, SO_REUSEADDR, &reuse, sizeof(reuse)) == -1)
    {
        perror("set reuseaddr | reuseport  socket failure !");

        exit(errno);
    }
}

/**
 * init epoll
 *
 * **/
int create_epoll()
{
    int epoll_fd = epoll_create1(EPOLL_CLOEXEC);

    // int epoll_fd = epoll_create(1);

    if (epoll_fd == -1)
    {
        perror("epoll_create1");

        exit(EXIT_FAILURE);
    }

    return epoll_fd;
}

#define MAX_EVENTS 10

struct epoll_event notify_events[MAX_EVENTS];

void add_epoll_event(int epoll_fd, int fd, struct epoll_event ev)
{
    if (-1 == epoll_ctl(epoll_fd, EPOLL_CTL_ADD, fd, &ev))
    {
        printf("epoll_ctl error, errno : %d \n", errno);
    }
}

void del_epoll_event(int epoll_fd, int fd)
{
    struct epoll_event ev = {0, {nullptr}};

    ev.data.ptr = nullptr;

    if (-1 == epoll_ctl(epoll_fd, EPOLL_CTL_DEL, fd, &ev))
    {
        printf("epoll_ctl error, errno : %d \n", errno);
    }
}

void mod_epoll_event(int epoll_fd, int fd, struct epoll_event ev)
{
    if (-1 == epoll_ctl(epoll_fd, EPOLL_CTL_MOD, fd, &ev))
    {
        printf("epoll_ctl error, errno : %d \n", errno);
    }
}

void do_use_fd(const struct epoll_event &ev);

class EventHandle
{
public:
    int epoll_fd;

    int fd;

    int events;

    int len;

    void *arg;

    EventHandle() = default;

    inline void send_callback() const
    {
        send(fd, arg, len, 0);

        bus.notify(Events::MyMessage{static_cast<char *>(arg)});

        //del fd from epoll
        del_epoll_event(epoll_fd, fd);

        //add fd to epoll
        struct epoll_event ev = {0, {nullptr}};

        ev.events = EPOLLIN | EPOLLET;

        ev.data.fd = fd;

        EventHandle evh{};

        evh.fd = fd;

        evh.epoll_fd = epoll_fd;

        evh.events = EPOLLIN | EPOLLET;

        ev.data.ptr = &evh;

        add_epoll_event(epoll_fd, fd, ev);

    }

    inline void recv_callback() const
    {
        char buf[BUFSIZ];

        bzero(buf, BUFSIZ);

        for (;;)
        {
            int ret = recv(fd, buf, BUFSIZ, 0);

            if (ret < 0)
            {

                if (errno == EINTR || errno == EWOULDBLOCK || errno == EAGAIN)
                    continue;

                printf("error\n");

                del_epoll_event(epoll_fd, fd);

                close(fd);

                break;
            } else if (ret == 0)
            {

                printf("客户端主动关闭请求！！！\n");

                del_epoll_event(epoll_fd, fd);

                close(fd);

                break;
            } else
            {
                write(STDOUT_FILENO, buf, ret);

                if (ret < BUFSIZ)
                {
                    printf("read finished \n");

                    std::transform(buf, buf + ret, buf, ToUpper<char>());

                    //del fd from epoll
                    del_epoll_event(epoll_fd, fd);

                    //add fd to epoll
                    struct epoll_event ev = {0, {nullptr}};

                    ev.events = EPOLLOUT | EPOLLET;

                    ev.data.fd = fd;

                    EventHandle evh;

                    evh.fd = fd;

                    evh.epoll_fd = epoll_fd;

                    evh.events = EPOLLOUT | EPOLLET;

                    evh.len = ret;

                    evh.arg = new char[ret];

                    memcpy(evh.arg, buf, ret);

                    ev.data.ptr = &evh;

                    add_epoll_event(epoll_fd, fd, ev);

                    break;
                }
            }
        }
    }

    inline void error_callback() const
    {
    }
};

#define MAX_LINE 10
// #define MAX_EVENTS 10
#define MAX_LISTENFD 5

int create_and_listen()
{
    int listenfd = socket(AF_INET, SOCK_STREAM, 0);

    struct sockaddr_in servaddr;

    // fcntl(listenfd, F_SETFL, O_NONBLOCK);
    set_nonblock(listenfd);

    // setsockopt(listenfd, SOL_SOCKET, SO_REUSEADDR, &on, sizeof(on));
    set_sock_reuse(listenfd);

    servaddr.sin_family = AF_INET;

    servaddr.sin_addr.s_addr = htonl(INADDR_ANY);

    servaddr.sin_port = htons(9999);

    if (-1 == bind(listenfd, (struct sockaddr *) &servaddr, sizeof(servaddr)))
    {
        printf("bind errno, errno : %d \n", errno);
    }

    if (-1 == listen(listenfd, MAX_LISTENFD))
    {
        printf("listen error, errno : %d \n", errno);
    }
    printf("listen in port 9999 !!!\n");

    return listenfd;
}

void test2()
{
    const int &sock_fd = create_and_listen();

    struct sockaddr_in acceptAddr{};

    socklen_t addr_len = sizeof(struct sockaddr);;

    const int &epoll_fd = create_epoll();

    struct epoll_event ev{};

    ev.data.fd = sock_fd;

    ev.events = EPOLLIN | EPOLLET;

    epoll_ctl(epoll_fd, EPOLL_CTL_ADD, sock_fd, &ev);

    for (;;)
    {
        int nfds = epoll_wait(epoll_fd, notify_events, MAX_EVENTS, -1); //时间参数为0表示立即返回，为-1表示无限等待

        if (nfds == -1)
        {
            perror("epoll_wait");
            exit(EXIT_FAILURE);
        }

        for (int n = 0; n < nfds; ++n)
        {
            if (notify_events[n].data.fd == sock_fd)
            {

                int conn_sock = accept4(sock_fd, (struct sockaddr *) &acceptAddr, &addr_len,
                                        SOCK_CLOEXEC | SOCK_NONBLOCK);

                // int conn_sock = accept(sock_fd, (struct sockaddr *)&acceptAddr, &addr_len);

                if (conn_sock == -1)
                {
                    perror("--->accept");

                    printf("sock_fd: %d \n errno: %d", sock_fd, errno);

                    exit(EXIT_FAILURE);
                }

                set_nonblock(conn_sock);

                ev.events = EPOLLIN | EPOLLET;

                ev.data.fd = conn_sock;

                EventHandle evh;

                evh.fd = conn_sock;

                evh.epoll_fd = epoll_fd;

                evh.events = EPOLLIN;

                ev.data.ptr = &evh;

                add_epoll_event(epoll_fd, conn_sock, ev);
            } else
            {
                do_use_fd(notify_events[n]);
            }
        }
    }

    close(sock_fd);

    for (auto &e : notify_events)
    {
        close(e.data.fd);
    }

    unlisten();

}

void do_use_fd(const struct epoll_event &ev)
{
    auto *evh = static_cast< EventHandle *>(ev.data.ptr);

    if (ev.events & EPOLLIN)
    {
        evh->recv_callback();
    } else if (ev.events & EPOLLOUT)
    {
        evh->send_callback();
    } else if (ev.events & EPOLLERR)
    {
        evh->error_callback();
    } else
    {
        //todo
    }
}

