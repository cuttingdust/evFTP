#include <event2/event.h>
#include <event2/listener.h>
#include <event2/bufferevent.h>
#include <event2/buffer.h>
#include <iostream>

#include <event2/bufferevent.h>
#include <event2/bufferevent.h>

#ifndef _WIN32
#include <signal.h>
#endif

#define SPORT 8080

bufferevent_filter_result filter_in(struct evbuffer *src, struct evbuffer *dst, ev_ssize_t dst_limit,
                                    enum bufferevent_flush_mode mode, void *arg)
{
    std::cout << "filter_in" << std::endl;

    char data[1024] = { 0 };
    /// 读取并清理原数据
    int len = evbuffer_remove(src, data, sizeof(data) - 1);

    /// 所有字母转成大写
    for (int i = 0; i < len; ++i)
    {
        data[i] = toupper(data[i]);
    }

    evbuffer_add(dst, data, len);

    return BEV_OK;
}

bufferevent_filter_result filter_out(struct evbuffer *src, struct evbuffer *dst, ev_ssize_t dst_limit,
                                     enum bufferevent_flush_mode mode, void *arg)
{
    std::cout << "filter_out" << std::endl;

    char data[1024] = { 0 };
    /// 读取并清理原数据
    int len = evbuffer_remove(src, data, sizeof(data) - 1);

    std::string str = "";
    str += "================\n";
    str += data;
    str += "================\n";

    evbuffer_add(dst, str.c_str(), str.size());

    return BEV_OK;
}


void read_cb(bufferevent *bev, void *arg)
{
    std::cout << "read_cb" << std::endl;
    char data[1024] = { 0 };
    int  len        = bufferevent_read(bev, data, sizeof(data) - 1);
    std::cout << data << std::endl;

    /// 回复客户消息，经过输出过滤
    bufferevent_write(bev, data, len);
}

void write_cb(bufferevent *bev, void *arg)
{
    std::cout << "write_cb" << std::endl;
    /// 打印写出的数据
}

void event_cb(bufferevent *bev, short event, void *args)
{
    std::cout << "event_cb" << std::endl;
}


/// typedef void (*evconnlistener_cb)(struct evconnlistener *, evutil_socket_t, struct sockaddr *, int socklen, void *);
void listen_cb(struct evconnlistener *e, evutil_socket_t s, struct sockaddr *a, int socklen, void *arg)
{
    std::cout << "listen_cb" << std::endl;
    event_base *base = (event_base *)arg;

    /// 创建bufferevent上下文
    bufferevent *bev = bufferevent_socket_new(base, s, BEV_OPT_CLOSE_ON_FREE);
    /// 绑定bufferevent filter
    bufferevent *bev_filter = bufferevent_filter_new(bev,                   /// bufferevent
                                                     filter_in,             /// 输入过滤函数
                                                     filter_out,            /// 输出过滤函数
                                                     BEV_OPT_CLOSE_ON_FREE, /// 关闭filter时同时关闭bufferevent
                                                     nullptr,               /// 清理的回调函数
                                                     nullptr                /// 传递给回调的参数
    );


    /// 设置bufferevent的回调
    bufferevent_setcb(bev_filter, read_cb, write_cb, event_cb, nullptr);

    /// 启用bufferevent的读写事件
    bufferevent_enable(bev_filter, EV_READ | EV_WRITE);
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

    std::cout << "server start!\n";
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

    return 0;
}
