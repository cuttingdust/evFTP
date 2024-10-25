#include <event2/event.h>

#ifndef _WIN32
#include <signal.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#endif

#include <iostream>
#include <thread>

#define SPORT 5001

void client_cb(evutil_socket_t s, short w, void *arg)
{
    /// 水平触发LT 只有有数据没有处理，会一直进入
    /// 边缘触发ET 有数据时只进入一次
    std::cout << "." << std::flush;
    return;

    // event *ev = (event *)arg;
    // /// 判断超时
    // if (w & EV_TIMEOUT)
    // {
    //     std::cout << "timeout" << std::endl;
    //     event_free(ev);
    //     evutil_closesocket(s);
    //     return;
    // }
    //
    // char buf[1024] = { 0 };
    // int  len       = recv(s, buf, sizeof(buf) - 1, 0);
    // if (len > 0)
    // {
    //     std::cout << buf << std::endl;
    //     send(s, "ok", 2, 0);
    // }
    // else
    // {
    //     /// 需要清理event
    //     std::cout << "event_free" << std::endl;
    //     event_free(ev);
    //     evutil_closesocket(s);
    // }
}


void listen_cb(evutil_socket_t s, short w, void *arg)
{
    std::cout << "listen_cb" << std::endl;
    sockaddr_in sin;
    socklen_t   size = sizeof(sin);

    /// 读取连接信息
    evutil_socket_t client = accept(s, (sockaddr *)&sin, &size);
    char            ip[16] = { 0 };
    evutil_inet_ntop(AF_INET, &sin.sin_addr, ip, sizeof(ip) - 1);
    std::cout << "client ip is " << ip << std::endl;

    /// 客户端数据读取事件
    event_base *base = (event_base *)arg;
    /// 水平触发LT 只有有数据没有处理，会一直进入
    // event  *ev = event_new(base, client, EV_READ | EV_PERSIST, client_cb, event_self_cbarg());
    event  *ev = event_new(base, client, EV_READ | EV_PERSIST | EV_ET, client_cb, event_self_cbarg());
    timeval t  = { 10, 0 };
    event_add(ev, &t);
}


int main(int argc, char *argv[])
{
    std::cout << "evsocket start!\n";
#ifdef _WIN32
    WSADATA wsaData;
    WSAStartup(MAKEWORD(2, 2), &wsaData);

#else
    /// 忽略管道信号，发送数据给已关闭的socket
    if (signal(SIGPIPE, SIG_IGN) == SIG_ERR)
        return 1;
#endif
    event_base *base = event_base_new();
    if (!base)
    {
        std::cerr << "event_base_new_with_config failed!" << std::endl;
        return -1;
    }

    /// 创建socket
    evutil_socket_t sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock <= 0)
    {
        std::cerr << "socket error!" << std::endl;
        return -1;
    }

    /// 设置地址复用和非阻塞
    evutil_make_socket_nonblocking(sock);
    evutil_make_listen_socket_reuseable(sock);

    /// 绑定端口和地址
    sockaddr_in sin;
    memset(&sin, 0, sizeof(sin));
    sin.sin_family = AF_INET;
    sin.sin_port   = htons(SPORT);
    int re         = ::bind(sock, (sockaddr *)&sin, sizeof(sin));
    if (re != 0)
    {
        std::cerr << "bind error!" << std::endl;
        return -1;
    }

    /// 开始监听
    ::listen(sock, 10);

    /// 开始接受连接请求 默认水平触发
    event *ev_listen = event_new(base, sock, EV_READ | EV_PERSIST, listen_cb, base);
    event_add(ev_listen, nullptr);

    /// 进入事件主循环
    event_base_dispatch(base);
    event_base_free(base);

    return 0;
}
