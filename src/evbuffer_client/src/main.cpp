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
    /// ��ȡ���뻺������
    int len = bufferevent_read(be, data, sizeof(data) - 1);
    // std::cout << "[" << data << "]" << std::endl;
    std::cout << data << std::flush;
    if (len <= 0)
        return;
    // if (strstr(data, "quit") != nullptr)
    // {
    //     std::cout << "quit" << std::endl;
    //     /// �˳����ر�socket BEV_OPT_CLOSE_ON_FREE
    //     bufferevent_free(be);
    // }

    recvstr += data;
    recvCount += len;

    /// �������� д�뵽�������
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
        /// ������β�����ļ�����
        fclose(fp);
        //�����������ܻ���ɻ�������û�з��ͽ���
        //bufferevent_free(be);
        bufferevent_disable(be, EV_WRITE);
        return;
    }

    sendCount += len;
    /// д��buffer
    bufferevent_write(be, data, len);
}


/// ���󣬳�ʱ �����ӶϿ�����룩
void event_cb(struct bufferevent *be, short events, void *arg)
{
    std::cout << "[E]" << std::flush;
    /// ��ȡ��ʱʱ�䷢�������ݶ�ȡֹͣ
    if (events & BEV_EVENT_TIMEOUT && events & BEV_EVENT_READING)
    {
        /// ��ȡ����������
        char data[1024] = { 0 };
        /// ��ȡ���뻺������
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

/// ���󣬳�ʱ �����ӶϿ�����룩
void client_event_cb(struct bufferevent *be, short events, void *arg)
{
    std::cout << "[client_E]" << std::flush;
    /// ��ȡ��ʱʱ�䷢�������ݶ�ȡֹͣ
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
        /// ���� write
        bufferevent_trigger(be, EV_WRITE, 0);
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
                             5,   /// ��ˮλ 0���������� Ĭ����0
                             10); /// ��ˮλ 0���������� Ĭ����0

    bufferevent_setwatermark(bev, EV_WRITE,
                             5,  /// ��ˮλ 0���������� Ĭ����0 �������ݵ���5 д��ص�������
                             0); /// ��ˮλ��Ч

    /// ��ʱʱ�������
    timeval t1 = { 0, 500000 };
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

    std::cout << "event buffer client start!\n";
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

    {
        /// ���ÿͻ��˴���
        /// -1 �ڲ�����socket
        bufferevent *bev = bufferevent_socket_new(base, -1, BEV_OPT_CLOSE_ON_FREE);
        sockaddr_in  sin;
        memset(&sin, 0, sizeof(sin));
        sin.sin_family = AF_INET;
        sin.sin_port   = htons(SPORT);
        evutil_inet_pton(AF_INET, "127.0.0.1", &sin.sin_addr.s_addr);
        FILE *fp = fopen("../../src/evbuffer_client/src/main.cpp", "rb");

        /// ���ûص�����
        bufferevent_setcb(bev, client_read_cb, client_write_cb, client_event_cb, fp);
        bufferevent_enable(bev, EV_READ | EV_WRITE);
        int re = bufferevent_socket_connect(bev, (sockaddr *)&sin, sizeof(sin));
        if (re == 0)
        {
            std::cout << "connected" << std::endl;
        }
    }

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
