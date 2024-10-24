#include <event2/event.h>
#include <iostream>
#include <thread>


static timeval t1 = { 1, 0 };
/// sock 文件描述符，which 事件类型 arg传递的参数
void timer1(evutil_socket_t sock, short which, void *arg)
{
    std::cout << "[timer1]" << std::flush;
    event *ev = (event *)arg;
    /// no pending
    if (!evtimer_pending(ev, &t1))
    {
        event_del(ev);
        event_add(ev, &t1);
    }
}

void timer2(evutil_socket_t sock, short which, void *arg)
{
    using namespace std::chrono_literals;
    std::cout << "[timer2]" << std::flush;
    std::this_thread::sleep_for(3000ms);
}

void timer3(evutil_socket_t sock, short which, void *arg)
{
    std::cout << "[timer3]" << std::flush;
}

int main(int argc, char *argv[])
{
    std::cout << "evtimer start!\n";
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

    /// event_new
    /*
	#define evtimer_new(b, cb, arg)		event_new((b), -1, 0, (cb), (arg))
	#define evtimer_add(ev, tv)		event_add((ev), (tv))
	#define evtimer_del(ev)			event_del(ev)
	*/

    ///定时器 非持久事件
    event *ev1 = evtimer_new(base, timer1, event_self_cbarg());
    if (!ev1)
    {
        std::cerr << "evtimer_new timer1 failed!" << std::endl;
        return -1;
    }
    evtimer_add(ev1, &t1); /// 插入性能 O(logn)

    static timeval t2;
    t2.tv_sec  = 1;
    t2.tv_usec = 200000; /// 微秒
    event *ev2 = event_new(base, -1, EV_PERSIST, timer2, 0);
    event_add(ev2, &t2);

    event *ev3 = event_new(base, -1, EV_PERSIST, timer3, ev1);
    /// 超时优化性能优化，默认event 用二叉堆存储（完全二叉树） 插入删除O（logn）
    /// 优化到双向队列 插入删除O（1）
    static timeval tv_in = { 3, 0 };
    const timeval *t3;
    t3 = event_base_init_common_timeout(base, &tv_in);
    event_add(ev3, t3); /// 插入性能 O(1)

    /// 进入事件主循环
    event_base_dispatch(base);
    event_base_free(base);

    return 0;
}
