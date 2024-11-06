#include "XThreadPool.h"
#include "XFTPServerCMD.h"

#include <event2/event.h>
#include <event2/listener.h>

#ifndef _WIN32
#include <signal.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#endif

#include <iostream>
#include <thread>


#define SPORT 8080

void listen_cb(struct evconnlistener *e, evutil_socket_t s, struct sockaddr *a, int socklen, void *arg)
{
    std::cout << "listen_cb" << std::endl;
    XTask *task = new XFTPServerCMD();
    task->sock_ = s;
    XThreadPool::getInstance()->dispatch(task);
}


int main(int argc, char *argv[])
{
    std::cout << "threadpool start!\n";
#ifdef _WIN32
    WSADATA wsaData;
    WSAStartup(MAKEWORD(2, 2), &wsaData);

#else
    /// ���Թܵ��źţ��������ݸ��ѹرյ�socket
    if (signal(SIGPIPE, SIG_IGN) == SIG_ERR)
        return 1;
#endif

    /// ��ʼ���̳߳�
    XThreadPool::getInstance()->init(10);

    event_base *base = event_base_new();
    if (!base)
    {
        std::cerr << "event_base_new_with_config failed!" << std::endl;
        return -1;
    }


    /// �󶨶˿ں͵�ַ
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
        evconnlistener_free(ev);
    if (base)
        event_base_free(base);

    return 0;
}
