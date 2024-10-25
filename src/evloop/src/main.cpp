#include <event2/event.h>
#include <iostream>
#include <thread>
#include <signal.h>

using namespace std::chrono_literals;


static bool isexit = false;

/// sock 文件描述符，which 事件类型 arg传递的参数
static void Ctrl_C(evutil_socket_t sock, short which, void *arg)
{
    std::cout << "Ctrl_C" << std::endl;

    event_base *base = (event_base *)arg;
    /// 执行完当前处理的事件函数就退出
    // event_base_loopbreak(base);

    /// 运行所有的活动事件再退出
    /// 事件循环没有运行时也要等运行一次再退出
    timeval t = { 3, 0 };
    event_base_loopexit(base, &t);

    std::cout << "Ctrl_C end" << std::endl;
}

static void Kill(evutil_socket_t sock, short which, void *arg)
{
    std::cout << "Kill" << std::endl;
    // std::this_thread::sleep_for(3s);
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
    std::cout << "evloop start!\n";
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
    // event_base_dispatch(base);
    /// EVLOOP_ONCE 等待一个事件运行，直到没有活动事件就退出返回0
    /// EVLOOP_NONBLOCK  有活动事件就处理，没有就返回0
    // while (!isexit)
    // {
    // event_base_loop(base, EVLOOP_NONBLOCK);
    // }

    // event_base_loop(base, EVLOOP_ONCE);
    event_base_loop(base, EVLOOP_NO_EXIT_ON_EMPTY);

    // event_free(csig);
    event_base_free(base);

    return 0;
}
