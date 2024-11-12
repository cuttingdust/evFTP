#include "XFTPTask.h"

#include <iostream>

#include <event2/bufferevent.h>
#include <event2/event.h>

auto XFTPTask::setCallback(struct bufferevent *bev) -> void
{
    bufferevent_setcb(bev, readCB, writeCB, eventCB, this);
    bufferevent_enable(bev, EV_READ | EV_WRITE);
}

auto XFTPTask::readCB(bufferevent *bev, void *arg) -> void
{
    XFTPTask *t = (XFTPTask *)arg;
    t->read(bev);
}

auto XFTPTask::writeCB(bufferevent *bev, void *arg) -> void
{
    XFTPTask *t = (XFTPTask *)arg;
    t->write(bev);
}

auto XFTPTask::eventCB(struct bufferevent *bev, short what, void *arg) -> void
{
    XFTPTask *t = (XFTPTask *)arg;
    t->event(bev, what);
}

auto XFTPTask::resCMD(std::string msg) -> void
{
    if (!cmdTask_ || !cmdTask_->bev_)
        return;

    if (msg[msg.size() - 1] != '\n')
    {
        msg += R"(\r\n)";
    }

    std::cout << "resCMD: " << msg << std::endl;

    bufferevent_write(cmdTask_->bev_, msg.c_str(), msg.size());
}

auto XFTPTask::connectPORT() -> void
{
    if (ip_.empty() || port_ <= 0 || !base_)
    {
        std::cout << "connectPORT failed ip or port or base is null" << std::endl;
        return;
    }

    if (bev_)
    {
        bufferevent_free(bev_);
        bev_ = 0;
    }
    bev_ = bufferevent_socket_new(base_, -1, BEV_OPT_CLOSE_ON_FREE);

    sockaddr_in sin;
    memset(&sin, 0, sizeof(sin));
    sin.sin_family = AF_INET;
    sin.sin_port   = htons(port_);
    evutil_inet_pton(AF_INET, ip_.c_str(), &sin.sin_addr.s_addr);

    /// 设置回调和权限
    setCallback(bev_);

    /// 添加超时
    timeval rt = { 600, 0 };
    bufferevent_set_timeouts(bev_, &rt, 0);
    bufferevent_socket_connect(bev_, (sockaddr *)&sin, sizeof(sin));
}

auto XFTPTask::close() -> void
{
    if (bev_)
    {
        bufferevent_free(bev_);
        bev_ = 0;
    }

    if (fp_)
    {
        fclose(fp_);
        fp_ = 0;
    }
}

auto XFTPTask::send(const std::string &data) -> void
{
    send(data.c_str(), data.size());
}

auto XFTPTask::send(const char *data, int dataSize) -> void
{
    if (!bev_)
        return;

    bufferevent_write(bev_, data, dataSize);
}
