#include <event2/event.h>
#include <event2/listener.h>
#include <event2/bufferevent.h>
#include <iostream>

#ifndef _WIN32
#include <signal.h>
#endif

#define SPORT 5001

void read_cb(struct bufferevent *be, void *arg)
{
    std::cout << "[R]" << std::flush;
    char data[1024] = { 0 };
    /// 读取输入缓冲数据
    int len = bufferevent_read(be, data, sizeof(data) - 1);
    std::cout << "[" << data << "]" << std::endl;
    if (len <= 0)
        return;
    if (strstr(data, "quit") != nullptr)
    {
        std::cout << "quit" << std::endl;
        /// 退出并关闭socket BEV_OPT_CLOSE_ON_FREE
        bufferevent_free(be);
    }

    /// 发送数据 写入到输出缓冲
    bufferevent_write(be, "OK", 3);
}

void write_cb(struct bufferevent *be, void *arg)
{
    std::cout << "[W]" << std::flush;
}

void event_cb(struct bufferevent *be, short events, void *arg)
{
    std::cout << "[E]" << std::flush;
}


void listen_cb(struct evconnlistener *e, evutil_socket_t s, struct sockaddr *a, int socklen, void *arg)
{
    std::cout << "listen_cb" << std::endl;
    event_base *base = (event_base *)arg;

    /// 创建bufferevent上下文 BEV_OPT_CLOSE_ON_FREE清理bufferevent时关闭socket
    bufferevent *bev = bufferevent_socket_new((struct event_base *)arg, s, BEV_OPT_CLOSE_ON_FREE);

    /// 添加监控事件
    bufferevent_enable(bev, EV_READ | EV_WRITE);

    /// 设置回调函数
    bufferevent_setcb(bev, read_cb, write_cb, event_cb, base);
}


int main(int argc, char *argv[])
{
#ifdef _WIN32
    WSADATA wsaData;
    WSAStartup(MAKEWORD(2, 2), &wsaData);

#else
    /// 忽略管道信号，发送数据给已关闭的socket
    if (signal(SIGPIPE, SIG_IGN) == SIG_ERR)
        return 1;
#endif

    std::cout << "event buffer server start!\n";
    ///创建libevent的上下文
    event_base *base = event_base_new();
    if (base)
    {
        std::cout << "event_base_new success!" << std::endl;
    }

    /// 监听端口
    /// socket, bind, listen 绑定事件
    sockaddr_in sin;
    memset(&sin, 0, sizeof(sin));
    sin.sin_family = AF_INET;
    sin.sin_port   = htons(SPORT);

    evconnlistener *ev = evconnlistener_new_bind(
            base,                                      ///  libevent的上下文
            listen_cb,                                 /// 接收到连接的回调函数
            base,                                      /// 回调函数获取的参数 arg
            LEV_OPT_REUSEABLE | LEV_OPT_CLOSE_ON_FREE, /// 地址重用，evconnlistener关闭同时关闭socket
            10,                                        /// 连接队列大小，对应listen函数
            (sockaddr *)&sin,                          /// 绑定的地址和端口
            sizeof(sin));

    /// 事件分发处理
    if (base)
        event_base_dispatch(base);
    if (ev)
    {
        evconnlistener_free(ev);
    }
    if (base)
    {
        event_base_free(base);
    }


#ifdef _WIN32
    WSACleanup();
#endif

    return 0;
}
