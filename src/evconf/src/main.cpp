#include <event2/event.h>
#include <event2/listener.h>
#include <iostream>

#ifndef _WIN32
#include <signal>
#endif

#define SPORT 5001

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

    std::cout << "evconf start!\n";
    /// ��������������
    event_config *conf = event_config_new();
    if (conf)
    {
        std::cout << "event_config_new success!" << std::endl;
    }

    /// ��ʾ֧�ֵ�����ģʽ
    const char **methods = event_get_supported_methods();
    std::cout << "supported_methods:" << std::endl;
    for (int i = 0; methods[i] != NULL; i++)
    {
        std::cout << methods[i] << std::endl;
    }

    ///����libevent��������
    event_base *base = event_base_new();
    if (!base)
    {
        std::cerr << "event_base_new failed!" << std::endl;
    }
    else
    {
        std::cout << "event_base_new success!" << std::endl;
        event_base_free(base);
    }

    /// �����˿�
    /// socket, bind, listen ���¼�
    sockaddr_in sin;
    memset(&sin, 0, sizeof(sin));
    sin.sin_family = AF_INET;
    sin.sin_port   = htons(SPORT);

#ifdef _WIN32
    WSACleanup();
#endif

    return 0;
}
