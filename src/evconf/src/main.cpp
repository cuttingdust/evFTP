#include <event2/event.h>
#include <event2/listener.h>
#include <iostream>

#ifndef _WIN32
#include <signal>
#endif

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

    ///初始化配置libevent的上下文
    event_base *base = event_base_new_with_config(conf);
    event_config_free(conf);

    if (!base)
    {
        std::cerr << "event_base_new_with_config failed!" << std::endl;
        base = event_base_new();
        if (!base)
        {
            std::cerr << "event_base_new failed!" << std::endl;
            return 0;
        }
    }
    else
    {
        /// 确认特征是否生效
        int f = event_base_get_features(base);
        if (f & EV_FEATURE_ET)
            std::cout << "EV_FEATURE_ET events are supported." << std::endl;
        else
            std::cout << "EV_FEATURE_ET events are not supported." << std::endl;

        if (f & EV_FEATURE_O1)
            std::cout << "EV_FEATURE_O1 events are supported." << std::endl;
        else
            std::cout << "EV_FEATURE_O1 events are not supported." << std::endl;

        if (f & EV_FEATURE_FDS)
            std::cout << "EV_FEATURE_FDS events are supported." << std::endl;
        else
            std::cout << "EV_FEATURE_FDS events are not supported." << std::endl;

        if (f & EV_FEATURE_EARLY_CLOSE)
            std::cout << "EV_FEATURE_EARLY_CLOSE events are supported." << std::endl;
        else
            std::cout << "EV_FEATURE_EARLY_CLOSE events are not supported." << std::endl;

        std::cout << "event_base_new_with_config success!" << std::endl;
        event_base_free(base);
    }

#ifdef _WIN32
    WSACleanup();
#endif

    return 0;
}
