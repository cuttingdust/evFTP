#include <event2/event.h>
#include <event2/listener.h>
#include <event2/bufferevent.h>
#include <iostream>

#include <event2/bufferevent.h>

#ifndef _WIN32
#include <signal.h>
#endif

#define SPORT 8080

static std::string recvstr   = "";
static int         recvCount = 0;
static int         sendCount = 0;

void read_cb(struct bufferevent *be, void *arg)
{
    std::cout << "[R]" << std::flush;
    char data[1024] = { 0 };
    /// 读取输入缓冲数据
    int len = bufferevent_read(be, data, sizeof(data) - 1);
    // std::cout << "[" << data << "]" << std::endl;
    std::cout << data << std::flush;
    if (len <= 0)
        return;
    // if (strstr(data, "quit") != nullptr)
    // {
    //     std::cout << "quit" << std::endl;
    //     /// 退出并关闭socket BEV_OPT_CLOSE_ON_FREE
    //     bufferevent_free(be);
    // }

    recvstr += data;
    recvCount += len;

    /// 发送数据 写入到输出缓冲
    bufferevent_write(be, "OK", 3);
}

void client_read_cb(bufferevent *be, void *arg)
{
    std::cout << "[client_R]" << std::flush;
}


void write_cb(struct bufferevent *be, void *arg)
{
    std::cout << "[W]" << std::flush;
}

void client_write_cb(bufferevent *be, void *arg)
{
    std::cout << "[client_W]" << std::flush;
    FILE *fp = (FILE *)arg;
    if (fp == nullptr)
    {
        return;
    }

    char data[1024] = { 0 };
    int  len        = fread(data, 1, sizeof(data) - 1, fp);
    if (len <= 0)
    {
        /// 读到结尾或者文件出错
        fclose(fp);
        //立刻清理，可能会造成缓冲数据没有发送结束
        //bufferevent_free(be);
        bufferevent_disable(be, EV_WRITE);
        return;
    }

    sendCount += len;
    /// 写入buffer
    bufferevent_write(be, data, len);
}


/// 错误，超时 （连接断开会进入）
void event_cb(struct bufferevent *be, short events, void *arg)
{
    std::cout << "[E]" << std::flush;
    /// 读取超时时间发生后，数据读取停止
    if (events & BEV_EVENT_TIMEOUT && events & BEV_EVENT_READING)
    {
        /// 读取缓冲中数据
        char data[1024] = { 0 };
        /// 读取输入缓冲数据
        int len = bufferevent_read(be, data, sizeof(data) - 1);
        if (len > 0)
        {
            recvCount += len;
            recvstr += data;
        }

        std::cout << "BEV_EVENT_READING BEV_EVENT_TIMEOUT" << std::endl;
        // bufferevent_enable(be,EV_READ);
        bufferevent_free(be);

        std::cout << recvstr << std::endl;
        std::cout << "recvCount=" << recvCount << " sendCount=" << sendCount << std::endl;
    }
    else if (events & BEV_EVENT_ERROR)
    {
        std::cout << "BEV_EVENT_ERROR" << std::endl;
        bufferevent_free(be);
    }
    else
    {
        std::cout << "OTHERS" << std::endl;
    }
}

/// 错误，超时 （连接断开会进入）
void client_event_cb(struct bufferevent *be, short events, void *arg)
{
    std::cout << "[client_E]" << std::flush;
    /// 读取超时时间发生后，数据读取停止
    if (events & BEV_EVENT_TIMEOUT && events & BEV_EVENT_READING)
    {
        // std::cout << "BEV_EVENT_READING BEV_EVENT_TIMEOUT" << std::endl;
        // bufferevent_enable(be,EV_READ);
        bufferevent_free(be);
    }
    else if (events & BEV_EVENT_ERROR)
    {
        std::cout << "BEV_EVENT_ERROR" << std::endl;
        bufferevent_free(be);
    }

    if (events & BEV_EVENT_EOF)
    {
        std::cout << "BEV_EVENT_EOF" << std::endl;
    }

    if (events & BEV_EVENT_CONNECTED)
    {
        std::cout << "BEV_EVENT_CONNECTED" << std::endl;
        /// 触发 write
        bufferevent_trigger(be, EV_WRITE, 0);
    }
}


void listen_cb(struct evconnlistener *e, evutil_socket_t s, struct sockaddr *a, int socklen, void *arg)
{
    std::cout << "listen_cb" << std::endl;
    event_base *base = (event_base *)arg;

    /// 创建bufferevent上下文 BEV_OPT_CLOSE_ON_FREE清理bufferevent时关闭socket
    bufferevent *bev = bufferevent_socket_new((struct event_base *)arg, s, BEV_OPT_CLOSE_ON_FREE);

    /// 添加监控事件
    bufferevent_enable(bev, EV_READ | EV_WRITE);

    /// 设置水位
    /// 读取水位
    bufferevent_setwatermark(bev, EV_READ,
                             5,   /// 低水位 0就是无限制 默认是0
                             10); /// 高水位 0就是无限制 默认是0

    bufferevent_setwatermark(bev, EV_WRITE,
                             5,  /// 低水位 0就是无限制 默认是0 缓冲数据低于5 写入回调被调用
                             0); /// 高水位无效

    /// 超时时间的设置
    timeval t1 = { 0, 500000 };
    bufferevent_set_timeouts(bev, &t1, nullptr);

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

    std::cout << "event buffer client start!\n";
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

    {
        /// 调用客户端代码
        /// -1 内部创建socket
        bufferevent *bev = bufferevent_socket_new(base, -1, BEV_OPT_CLOSE_ON_FREE);
        sockaddr_in  sin;
        memset(&sin, 0, sizeof(sin));
        sin.sin_family = AF_INET;
        sin.sin_port   = htons(SPORT);
        evutil_inet_pton(AF_INET, "127.0.0.1", &sin.sin_addr.s_addr);
        FILE *fp = fopen("../../src/evbuffer_client/src/main.cpp", "rb");

        /// 设置回调函数
        bufferevent_setcb(bev, client_read_cb, client_write_cb, client_event_cb, fp);
        bufferevent_enable(bev, EV_READ | EV_WRITE);
        int re = bufferevent_socket_connect(bev, (sockaddr *)&sin, sizeof(sin));
        if (re == 0)
        {
            std::cout << "connected" << std::endl;
        }
    }

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
