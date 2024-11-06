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
    /// 忽略管道信号，发送数据给已关闭的socket
    if (signal(SIGPIPE, SIG_IGN) == SIG_ERR)
        return 1;
#endif

    /// 初始化线程池
    XThreadPool::getInstance()->init(10);

    event_base *base = event_base_new();
    if (!base)
    {
        std::cerr << "event_base_new_with_config failed!" << std::endl;
        return -1;
    }


    /// 绑定端口和地址
    sockaddr_in sin;
    memset(&sin, 0, sizeof(sin));
    sin.sin_family = AF_INET;
    sin.sin_port   = htons(SPORT);

    evconnlistener *ev = evconnlistener_new_bind(
            base,                                      ///  libevent的上下文
            listen_cb,                                 /// 接收到连接的回调函数
            base,                                      /// 回调函数获取的参数 arg
            LEV_OPT_REUSEABLE | LEV_OPT_CLOSE_ON_FREE, /// 地址重用，evconnlistener关闭同时关闭socket
            10,                                        /// 连接队列大小，对应listen函数
            (sockaddr *)&sin,                          /// 绑定的地址和端口
            sizeof(sin));


    /// 事件分发处理
    if (base)
        event_base_dispatch(base);
    if (ev)
        evconnlistener_free(ev);
    if (base)
        event_base_free(base);

    return 0;
}
