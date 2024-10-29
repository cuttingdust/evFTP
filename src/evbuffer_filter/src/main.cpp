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
    /// ��ȡ������ԭ����
    int len = evbuffer_remove(src, data, sizeof(data) - 1);

    /// ������ĸת�ɴ�д
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
    /// ��ȡ������ԭ����
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

    /// �ظ��ͻ���Ϣ�������������
    bufferevent_write(bev, data, len);
}

void write_cb(bufferevent *bev, void *arg)
{
    std::cout << "write_cb" << std::endl;
    /// ��ӡд��������
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

    /// ����bufferevent������
    bufferevent *bev = bufferevent_socket_new(base, s, BEV_OPT_CLOSE_ON_FREE);
    /// ��bufferevent filter
    bufferevent *bev_filter = bufferevent_filter_new(bev,                   /// bufferevent
                                                     filter_in,             /// ������˺���
                                                     filter_out,            /// ������˺���
                                                     BEV_OPT_CLOSE_ON_FREE, /// �ر�filterʱͬʱ�ر�bufferevent
                                                     nullptr,               /// ����Ļص�����
                                                     nullptr                /// ���ݸ��ص��Ĳ���
    );


    /// ����bufferevent�Ļص�
    bufferevent_setcb(bev_filter, read_cb, write_cb, event_cb, nullptr);

    /// ����bufferevent�Ķ�д�¼�
    bufferevent_enable(bev_filter, EV_READ | EV_WRITE);
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

    std::cout << "server start!\n";
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

    return 0;
}
