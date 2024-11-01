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

    void Server(event_base * base);
    Server(base);

    void Client(event_base * base);
    Client(base);

    /// �¼��ַ�����
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
