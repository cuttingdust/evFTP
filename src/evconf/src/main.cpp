#include <event2/event.h>
#include <event2/listener.h>
#include <event2/thread.h>
#include <iostream>

#ifndef _WIN32
#include <signal.h>
#endif

#define SPORT 8080

void listen_cb(evconnlistener *ev, evutil_socket_t s, struct sockaddr *addr, int socklen, void *arg)
{
    std::cout << "lesson_cb" << std::endl;
}


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


    ///设置网络模型
    // event_config_avoid_method(conf, "select");
    // event_config_avoid_method(conf, "wepoll");
    // event_config_avoid_method(conf, "win32");
    // event_config_avoid_method(conf, "kqueue");
    // event_config_avoid_method(conf, "epoll");
    // event_config_avoid_method(conf, "poll");

    /// windows 中支持IOCP (线程池)
#ifdef _WIN32
    event_config_set_flag(conf, EVENT_BASE_FLAG_STARTUP_IOCP);
    /// 初始化iocp的线程
    evthread_use_windows_threads();
    /// 设置cpu数量
    SYSTEM_INFO si;
    GetSystemInfo(&si);
    event_config_set_num_cpus_hint(conf, si.dwNumberOfProcessors);
#endif


#ifndef _WIN32
    /// 设置特征
    /// 设置了EV_FEATURE_FDS 其他特征就无法设置，在windows中EV_FEATURE_FDS无效
    /// 仅仅限制kqueue epoll
    event_config_require_features(conf, EV_FEATURE_ET | EV_FEATURE_FDS);
    // event_config_require_features(conf, EV_FEATURE_FDS);
#endif

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
        /// 获取当前的网络模型
        std::cout << "current method: " << event_base_get_method(base) << std::endl;


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


        sockaddr_in sin;
        memset(&sin, 0, sizeof(sin));
        sin.sin_family = AF_INET;
        sin.sin_port   = htons(SPORT);

        evconnlistener *ev = evconnlistener_new_bind(
                base, listen_cb, base, 10, LEV_OPT_CLOSE_ON_FREE | LEV_OPT_REUSEABLE, (sockaddr *)&sin, sizeof(sin));

        event_base_dispatch(base);
        evconnlistener_free(ev);
        event_base_free(base);
    }


#ifdef _WIN32
    WSACleanup();
#endif

    return 0;
}
