#include "XFTPServerCMD.h"
#include "XFTPTask.h"

#include <event2/bufferevent.h>
#include <event2/event.h>

#include <iostream>

XFTPServerCMD::XFTPServerCMD()
{
}

XFTPServerCMD::~XFTPServerCMD()
{
}

/// 注册命令处理对象 不需要考虑线程安全 调用时还未分发到线程
auto XFTPServerCMD::reg(const std::string& cmd, XFTPTask* call) -> void
{
    if (!call)
    {
        std::cout << "XFTPServerCMD::reg() call is null" << std::endl;
        return;
    }
    if (cmd.empty())
    {
        std::cout << "XFTPServerCMD::reg() cmd is null" << std::endl;
        return;
    }

    std::cout << "XFTPServerCMD::reg()" << std::endl;

    /// 已经注册的是否覆盖 不覆盖，提示错误
    if (calls_.find(cmd) != calls_.end())
    {
        std::cout << cmd << " is already register" << std::endl;
        return;
    }
    calls_.emplace(cmd, call);
}

auto XFTPServerCMD::init() -> bool
{
    std::cout << "XFTPServerCMD::init()" << std::endl;
    /// 监听socket bufferevent
    /// base socket

    bufferevent* bev = bufferevent_socket_new(base_, sock_, BEV_OPT_CLOSE_ON_FREE);
    this->bev_       = bev;
    this->setCallback(bev);

    /// 添加超时
    timeval rt_ = { 60, 0 };
    bufferevent_set_timeouts(bev, &rt_, 0);

    std::string msg = "220 Welcome to libevent XFtpServer\r\n";
    bufferevent_write(bev, msg.c_str(), msg.size());

    return true;
}

/// 子线程XThread  event事件分发
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
        /// 分析出类型 USER anonymous
        std::string type = "";
        for (int i = 0; i < len; i++)
        {
            if (data[i] == ' ' || data[i] == '\r')
                break;
            type += data[i];
        }
        std::cout << "type is [" << type << "]" << std::endl;
        if (calls_.find(type) != calls_.end())
        {
            XFTPTask* t = calls_[type];
            t->cmdTask_ = this; /// 用来处理回复命令和目录
            t->parse(type, data);
        }
        else
        {
            std::string msg = "200 OK\r\n";
            bufferevent_write(bev, msg.c_str(), msg.size());
        }
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
