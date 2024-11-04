#include <event2/event.h>
#include <iostream>
#include <thread>
#include <signal.h>

using namespace std::chrono_literals;


static bool isexit = false;

/// sock �ļ���������which �¼����� arg���ݵĲ���
static void Ctrl_C(evutil_socket_t sock, short which, void *arg)
{
    std::cout << "Ctrl_C" << std::endl;

    event_base *base = (event_base *)arg;
    /// ִ���굱ǰ�������¼��������˳�
    // event_base_loopbreak(base);

    /// �������еĻ�¼����˳�
    /// �¼�ѭ��û������ʱҲҪ������һ�����˳�
    timeval t = { 3, 0 };
    event_base_loopexit(base, &t);

    std::cout << "Ctrl_C end" << std::endl;
}

static void Kill(evutil_socket_t sock, short which, void *arg)
{
    std::cout << "Kill" << std::endl;
    // std::this_thread::sleep_for(3s);
    event *ev = (event *)arg;
    /// ������ڷǴ���
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
    /// ���Թܵ��źţ��������ݸ��ѹرյ�socket
    if (signal(SIGPIPE, SIG_IGN) == SIG_ERR)
        return 1;
#endif


    event_base *base = event_base_new();
    if (base)
    {
        std::cout << "event_base_new success!" << std::endl;
    }

    /// ����ctrl +C �ź��¼�������no pending
    /// evsignal_new ���ص�״̬ EV_SIGNAL|EV_PERSIST
    event *csig = evsignal_new(base, SIGINT, Ctrl_C, base);
    if (!csig)
    {
        std::cerr << "SIGINT evsignal_new failed!" << std::endl;
        return -1;
    }

    /// �����¼���pending
    if (event_add(csig, 0) != 0)
    {
        std::cerr << "SIGINT event_add failed!" << std::endl;
        return -1;
    }

    /// ����kill�ź�
    ///  �ǳ־��¼���ֻ����һ�� event_self_cbarg()���ݵ�ǰ��event
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


    /// �����¼���ѭ��
    // event_base_dispatch(base);
    /// EVLOOP_ONCE �ȴ�һ���¼����У�ֱ��û�л�¼����˳�����0
    /// EVLOOP_NONBLOCK  �л�¼��ʹ�����û�оͷ���0
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