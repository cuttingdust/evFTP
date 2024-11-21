#include "XMsgServer.h"

#include <iostream>

#include <event2/event.h>
#include <event2/listener.h>
#include <event2/bufferevent.h>

static void read_cb(struct bufferevent *be, void *arg)
{
    std::cout << "[SR]" << std::flush;
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

/// 错误，超时 （连接断开会进入）
static void event_cb(struct bufferevent *be, short events, void *arg)
{
    std::cout << "[E]" << std::flush;

    /// 读取超时时间发生后，数据读取停止
    if (events & (BEV_EVENT_TIMEOUT | BEV_EVENT_READING))
    {
        std::cout << "BEV_EVENT_READING BEV_EVENT_TIMEOUT" << std::endl;
        bufferevent_free(be);
    }
    else if (events & (BEV_EVENT_ERROR | BEV_EVENT_EOF))
    {
        std::cout << "BEV_EVENT_ERROR  | BEV_EVENT_EOF" << std::endl;
        bufferevent_free(be);
    }
    else
    {
        std::cout << "OTHERS" << std::endl;
    }
}

static void listen_cb(struct evconnlistener *evc, evutil_socket_t client_socket, struct sockaddr *client_addr,
                      int socklen, void *arg)
{
    std::cout << "listen_cb" << std::endl;

    char         ip[16] = { 0 };
    sockaddr_in *addr   = (sockaddr_in *)client_addr;
    evutil_inet_ntop(AF_INET, &addr->sin_addr, ip, sizeof(ip));
    std::cout << "client ip is " << ip << std::endl;

    event_base *base = (event_base *)arg;

    /// 创建bufferevent上下文 BEV_OPT_CLOSE_ON_FREE清理bufferevent时关闭socket
    bufferevent *bev = bufferevent_socket_new(base, client_socket, BEV_OPT_CLOSE_ON_FREE);
    if (!bev)
    {
        std::cerr << "bufferevent_socket_new failed!" << std::endl;
        return;
    }

    /// 添加监控事件
    bufferevent_enable(bev, EV_READ | EV_WRITE);

    /// 超时时间的设置
    timeval t1 = { 30, 0 };
    bufferevent_set_timeouts(bev, &t1, nullptr);

    /// 设置回调函数
    bufferevent_setcb(bev, read_cb, 0, event_cb, base);
}

XMsgServer::XMsgServer() = default;

XMsgServer::~XMsgServer() = default;

void XMsgServer::initServer(int serverPort)
{
    std::cout << "XMsgServer::initServer" << std::endl;
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
    sin.sin_port   = htons(serverPort);

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
        evconnlistener_free(ev);
    if (base)
        event_base_free(base);
}
