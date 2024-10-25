#include <event2/event.h>
#include <iostream>
#include <signal.h>

/// sock 文件描述符，which 事件类型 arg传递的参数
static void Ctrl_C(evutil_socket_t sock, short which, void *arg)
{
    std::cout << "Ctrl_C" << std::endl;
}

static void Kill(evutil_socket_t sock, short which, void *arg)
{
    std::cout << "Kill" << std::endl;

    event *ev = (event *)arg;
    /// 如果处于非待决
    if (!evsignal_pending(ev, NULL))
    {
        std::cout << "event_del and event_add " << __func__ << std::endl;
        event_del(ev);
        event_add(ev, NULL);
    }
}


int main(int argc, char *argv[])
{
    std::cout << "evsignal start!\n";
#ifdef _WIN32
    WSADATA wsaData;
    WSAStartup(MAKEWORD(2, 2), &wsaData);

#else
    /// 忽略管道信号，发送数据给已关闭的socket
    if (signal(SIGPIPE, SIG_IGN) == SIG_ERR)
        return 1;
#endif


    event_base *base = event_base_new();
    if (base)
    {
        std::cout << "event_base_new success!" << std::endl;
    }

    /// 添加ctrl +C 信号事件，处于no pending
    /// evsignal_new 隐藏的状态 EV_SIGNAL|EV_PERSIST
    event *csig = evsignal_new(base, SIGINT, Ctrl_C, base);
    if (!csig)
    {
        std::cerr << "SIGINT evsignal_new failed!" << std::endl;
        return -1;
    }

    /// 添加事件到pending
    if (event_add(csig, 0) != 0)
    {
        std::cerr << "SIGINT event_add failed!" << std::endl;
        return -1;
    }

    /// 添加kill信号
    ///  非持久事件，只进入一次 event_self_cbarg()传递当前的event
    event *ksig = event_new(base, SIGTERM, EV_SIGNAL, Kill, event_self_cbarg());
    if (!ksig)
    {
        std::cerr << "SIGTERM evsignal_new failed!" << std::endl;
        return -1;
    }

    if (event_add(ksig, 0) != 0)
    {
        std::cerr << "SIGTERM event_add failed!" << std::endl;
        return -1;
    }


    /// 进入事件主循环
    event_base_dispatch(base);
    event_free(csig);
    event_base_free(base);

    return 0;
}
