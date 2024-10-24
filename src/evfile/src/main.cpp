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

void read_file(evutil_socket_t fd, short event, void *arg)
{
    // char buf[1024] = { 0 };
    // int  len       = ::read(fd, buf, sizeof(buf) - 1);
    // if (len > 0)
    // {
    //     std::cout << buf << std::endl;
    // }
    // else
    // {
    //     std::cout << "." << std::flush;
    //     std::this_thread::sleep_for(std::chrono::milliseconds(500));
    // }

    std::cout << "read_file" << std::endl;
}

int main(int argc, char *argv[])
{
    std::cout << "evfile start!\n";
#ifdef _WIN32
    WSADATA wsaData;
    WSAStartup(MAKEWORD(2, 2), &wsaData);

#else
    /// ���Թܵ��źţ��������ݸ��ѹرյ�socket
    if (signal(SIGPIPE, SIG_IGN) == SIG_ERR)
        return 1;
#endif
    event_config *conf = event_config_new();
    /// ����֧���ļ�������
    event_base *base = event_base_new_with_config(conf);
    event_config_free(conf);
    if (!base)
    {
        std::cerr << "event_base_new_with_config failed!" << std::endl;
        return -1;
    }

    /// ���ļ�ֻ����������
    int sock = ::open("/var/log/system.log", O_RDONLY | O_NONBLOCK, 0);
    if (sock <= 0)
    {
        std::cerr << "open /var/log/system.log failed!" << std::endl;
        return -2;
    }

    /// �ļ�ָ���Ƶ���β��
    ::lseek(sock, 0, SEEK_END);

    ///�����ļ�����
    event *ev = event_new(base, sock, EV_READ | EV_PERSIST, read_file, nullptr);
    event_add(ev, nullptr);

    /// �����¼���ѭ��
    event_base_dispatch(base);
    event_base_free(base);

    return 0;
}
