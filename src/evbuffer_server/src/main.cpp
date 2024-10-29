#include <event2/event.h>
#include <event2/listener.h>
#include <event2/bufferevent.h>
#include <iostream>

#ifndef _WIN32
#include <signal.h>
#endif

#define SPORT 8080

void read_cb(struct bufferevent *be, void *arg)
{
    std::cout << "[R]" << std::flush;
    char data[1024] = { 0 };
    /// ��ȡ���뻺������
    int len = bufferevent_read(be, data, sizeof(data) - 1);
    std::cout << "[" << data << "]" << std::endl;
    if (len <= 0)
        return;
    if (strstr(data, "quit") != nullptr)
    {
        std::cout << "quit" << std::endl;
        /// �˳����ر�socket BEV_OPT_CLOSE_ON_FREE
        bufferevent_free(be);
    }

    /// �������� д�뵽�������
    bufferevent_write(be, "OK", 3);
}

void write_cb(struct bufferevent *be, void *arg)
{
    std::cout << "[W]" << std::flush;
}

/// ���󣬳�ʱ �����ӶϿ�����룩
void event_cb(struct bufferevent *be, short events, void *arg)
{
    std::cout << "[E]" << std::flush;
    /// ��ȡ��ʱʱ�䷢�������ݶ�ȡֹͣ
    if (events & BEV_EVENT_TIMEOUT && events & BEV_EVENT_READING)
    {
        std::cout << "BEV_EVENT_READING BEV_EVENT_TIMEOUT" << std::endl;
        // bufferevent_enable(be,EV_READ);
        bufferevent_free(be);
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


void listen_cb(struct evconnlistener *e, evutil_socket_t s, struct sockaddr *a, int socklen, void *arg)
{
    std::cout << "listen_cb" << std::endl;
    event_base *base = (event_base *)arg;

    /// ����bufferevent������ BEV_OPT_CLOSE_ON_FREE����buffereventʱ�ر�socket
    bufferevent *bev = bufferevent_socket_new((struct event_base *)arg, s, BEV_OPT_CLOSE_ON_FREE);

    /// ��Ӽ���¼�
    bufferevent_enable(bev, EV_READ | EV_WRITE);

    /// ����ˮλ
    /// ��ȡˮλ
    bufferevent_setwatermark(bev, EV_READ,
        5,    /// ��ˮλ 0���������� Ĭ����0
        10); /// ��ˮλ 0���������� Ĭ����0

    bufferevent_setwatermark(bev, EV_WRITE,
        5,      /// ��ˮλ 0���������� Ĭ����0 �������ݵ���5 д��ص�������
        0);    /// ��ˮλ��Ч

    /// ��ʱʱ�������
    timeval t1 = { 3, 0 };
    bufferevent_set_timeouts(bev, &t1, nullptr);

    /// ���ûص�����
    bufferevent_setcb(bev, read_cb, write_cb, event_cb, base);
}


int main(int argc, char *argv[])
{
#ifdef _WIN32
    WSADATA wsaData;
    WSAStartup(MAKEWORD(2, 2), &wsaData);

#else
    /// ���Թܵ��źţ��������ݸ��ѹرյ�socket
    if (signal(SIGPIPE, SIG_IGN) == SIG_ERR)
        return 1;
#endif

    std::cout << "event buffer server start!\n";
    ///����libevent��������
    event_base *base = event_base_new();
    if (base)
    {
        std::cout << "event_base_new success!" << std::endl;
    }

    /// �����˿�
    /// socket, bind, listen ���¼�
    sockaddr_in sin;
    memset(&sin, 0, sizeof(sin));
    sin.sin_family = AF_INET;
    sin.sin_port   = htons(SPORT);

    evconnlistener *ev = evconnlistener_new_bind(
            base,                                      ///  libevent��������
            listen_cb,                                 /// ���յ����ӵĻص�����
            base,                                      /// �ص�������ȡ�Ĳ��� arg
            LEV_OPT_REUSEABLE | LEV_OPT_CLOSE_ON_FREE, /// ��ַ���ã�evconnlistener�ر�ͬʱ�ر�socket
            10,                                        /// ���Ӷ��д�С����Ӧlisten����
            (sockaddr *)&sin,                          /// �󶨵ĵ�ַ�Ͷ˿�
            sizeof(sin));

    /// �¼��ַ�����
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
