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
    /// 忽略管道信号，发送数据给已关闭的socket
    if (signal(SIGPIPE, SIG_IGN) == SIG_ERR)
        return 1;
#endif

    std::cout << "evconf start!\n";
    /// 创建配置上下文
    event_config *conf = event_config_new();
    if (conf)
    {
        std::cout << "event_config_new success!" << std::endl;
    }

    /// 显示支持的网络模式
    const char **methods = event_get_supported_methods();
    std::cout << "supported_methods:" << std::endl;
    for (int i = 0; methods[i] != NULL; i++)
    {
        std::cout << methods[i] << std::endl;
    }

    ///创建libevent的上下文
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

    /// 监听端口
    /// socket, bind, listen 绑定事件
    sockaddr_in sin;
    memset(&sin, 0, sizeof(sin));
    sin.sin_family = AF_INET;
    sin.sin_port   = htons(SPORT);

#ifdef _WIN32
    WSACleanup();
#endif

    return 0;
}
