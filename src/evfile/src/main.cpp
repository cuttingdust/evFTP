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
    /// 忽略管道信号，发送数据给已关闭的socket
    if (signal(SIGPIPE, SIG_IGN) == SIG_ERR)
        return 1;
#endif
    event_config *conf = event_config_new();
    /// 设置支持文件描述符
    event_base *base = event_base_new_with_config(conf);
    event_config_free(conf);
    if (!base)
    {
        std::cerr << "event_base_new_with_config failed!" << std::endl;
        return -1;
    }

    /// 打开文件只读，非阻塞
    int sock = ::open("/var/log/system.log", O_RDONLY | O_NONBLOCK, 0);
    if (sock <= 0)
    {
        std::cerr << "open /var/log/system.log failed!" << std::endl;
        return -2;
    }

    /// 文件指针移到结尾处
    ::lseek(sock, 0, SEEK_END);

    ///监听文件数据
    event *ev = event_new(base, sock, EV_READ | EV_PERSIST, read_file, nullptr);
    event_add(ev, nullptr);

    /// 进入事件主循环
    event_base_dispatch(base);
    event_base_free(base);

    return 0;
}
