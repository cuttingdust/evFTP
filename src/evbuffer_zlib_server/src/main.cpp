#include <event2/event.h>
#include <iostream>

#ifndef _WIN32
#include <signal.h>
#endif


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

    void Server(event_base * base);
    Server(base);

    void Client(event_base * base);
    Client(base);

    /// 事件分发处理
    if (base)
    {
        event_base_dispatch(base);
    }

    if (base)
    {
        event_base_free(base);
    }

    return 0;
}
