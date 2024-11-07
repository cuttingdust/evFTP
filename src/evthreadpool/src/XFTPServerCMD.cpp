#include "XFTPServerCMD.h"

#include <event2/bufferevent.h>
#include <event2/event.h>

#include <iostream>

XFTPServerCMD::XFTPServerCMD()
{
}

XFTPServerCMD::~XFTPServerCMD()
{
}

auto XFTPServerCMD::init() -> bool
{
    std::cout << "XFTPServerCMD::init()" << std::endl;
    /// 监听socket bufferevent
    /// base socket

    bufferevent* bev = bufferevent_socket_new(base_, sock_, BEV_OPT_CLOSE_ON_FREE);
    this->setCallback(bev);

    /// 添加超时
    timeval rt_ = { 60, 0 };
    bufferevent_set_timeouts(bev, &rt_, 0);

    return true;
}

auto XFTPServerCMD::read(struct bufferevent* bev) -> void
{
    char data[1024] = { 0 };
    for (;;)
    {
        int len = bufferevent_read(bev, data, sizeof(data) - 1);
        if (len <= 0)
            break;
        data[len] = '\0';
        std::cout << "Recv CMD:" << data << std::flush;
        /// 分发到处理对象
    }
}

auto XFTPServerCMD::event(struct bufferevent* bev, short what) -> void
{
    std::cout << "event_cb" << std::endl;

    /// 如果对方网络断掉，或者机器死机有可能收不到BEV_EVENT_EOF数据
    if (what & (BEV_EVENT_EOF | BEV_EVENT_ERROR | BEV_EVENT_TIMEOUT))
    {
        std::cout << "BEV_EVENT_EOF | BEV_EVENT_ERROR | BEV_EVENT_TIMEOUT" << std::endl;
        bufferevent_free(bev);
        delete this;
    }
}
