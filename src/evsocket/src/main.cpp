#include <event2/event.h>

#ifndef _WIN32
#include <signal.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#endif

#include <iostream>
#include <thread>

#define SPORT 5001

void client_cb(evutil_socket_t s, short w, void *arg)
{
    /// ˮƽ����LT ֻ��������û�д�����һֱ����
    /// ��Ե����ET ������ʱֻ����һ��
    std::cout << "." << std::flush;
    return;

    // event *ev = (event *)arg;
    // /// �жϳ�ʱ
    // if (w & EV_TIMEOUT)
    // {
    //     std::cout << "timeout" << std::endl;
    //     event_free(ev);
    //     evutil_closesocket(s);
    //     return;
    // }
    //
    // char buf[1024] = { 0 };
    // int  len       = recv(s, buf, sizeof(buf) - 1, 0);
    // if (len > 0)
    // {
    //     std::cout << buf << std::endl;
    //     send(s, "ok", 2, 0);
    // }
    // else
    // {
    //     /// ��Ҫ����event
    //     std::cout << "event_free" << std::endl;
    //     event_free(ev);
    //     evutil_closesocket(s);
    // }
}


void listen_cb(evutil_socket_t s, short w, void *arg)
{
    std::cout << "listen_cb" << std::endl;
    sockaddr_in sin;
    socklen_t   size = sizeof(sin);

    /// ��ȡ������Ϣ
    evutil_socket_t client = accept(s, (sockaddr *)&sin, &size);
    char            ip[16] = { 0 };
    evutil_inet_ntop(AF_INET, &sin.sin_addr, ip, sizeof(ip) - 1);
    std::cout << "client ip is " << ip << std::endl;

    /// �ͻ������ݶ�ȡ�¼�
    event_base *base = (event_base *)arg;
    /// ˮƽ����LT ֻ��������û�д�����һֱ����
    // event  *ev = event_new(base, client, EV_READ | EV_PERSIST, client_cb, event_self_cbarg());
    event  *ev = event_new(base, client, EV_READ | EV_PERSIST | EV_ET, client_cb, event_self_cbarg());
    timeval t  = { 10, 0 };
    event_add(ev, &t);
}


int main(int argc, char *argv[])
{
    std::cout << "evsocket start!\n";
#ifdef _WIN32
    WSADATA wsaData;
    WSAStartup(MAKEWORD(2, 2), &wsaData);

#else
    /// ���Թܵ��źţ��������ݸ��ѹرյ�socket
    if (signal(SIGPIPE, SIG_IGN) == SIG_ERR)
        return 1;
#endif
    event_base *base = event_base_new();
    if (!base)
    {
        std::cerr << "event_base_new_with_config failed!" << std::endl;
        return -1;
    }

    /// ����socket
    evutil_socket_t sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock <= 0)
    {
        std::cerr << "socket error!" << std::endl;
        return -1;
    }

    /// ���õ�ַ���úͷ�����
    evutil_make_socket_nonblocking(sock);
    evutil_make_listen_socket_reuseable(sock);

    /// �󶨶˿ں͵�ַ
    sockaddr_in sin;
    memset(&sin, 0, sizeof(sin));
    sin.sin_family = AF_INET;
    sin.sin_port   = htons(SPORT);
    int re         = ::bind(sock, (sockaddr *)&sin, sizeof(sin));
    if (re != 0)
    {
        std::cerr << "bind error!" << std::endl;
        return -1;
    }

    /// ��ʼ����
    ::listen(sock, 10);

    /// ��ʼ������������ Ĭ��ˮƽ����
    event *ev_listen = event_new(base, sock, EV_READ | EV_PERSIST, listen_cb, base);
    event_add(ev_listen, nullptr);

    /// �����¼���ѭ��
    event_base_dispatch(base);
    event_base_free(base);

    return 0;
}
