#include "XFTPServerCMD.h"

#include <event2/bufferevent.h>
#include <event2/event.h>

#include <iostream>

void event_cb(struct bufferevent* bev, short what, void* ctx)
{
    std::cout << "event_cb" << std::endl;
    auto* cmd = (XFTPServerCMD*)ctx;

    /// ����Է�����ϵ������߻��������п����ղ���BEV_EVENT_EOF����
    if (what & (BEV_EVENT_EOF | BEV_EVENT_ERROR | BEV_EVENT_TIMEOUT))
    {
        std::cout << "BEV_EVENT_EOF | BEV_EVENT_ERROR | BEV_EVENT_TIMEOUT" << std::endl;
        bufferevent_free(bev);
        delete cmd;
    }
}


static void read_cb(bufferevent* bev, void* arg)
{
    auto* cmd        = (XFTPServerCMD*)arg;
    char  data[1024] = { 0 };
    for (;;)
    {
        int len = bufferevent_read(bev, data, sizeof(data) - 1);
        if (len <= 0)
            break;
        data[len] = '\0';
        std::cout << data << std::flush;

        ///TODO ���Դ��룬Ҫ�����
        if (strstr(data, "quit"))
        {
            bufferevent_free(bev);
            delete cmd;
            break;
        }
    }
}

XFTPServerCMD::XFTPServerCMD()
{
}

XFTPServerCMD::~XFTPServerCMD()
{
}

auto XFTPServerCMD::init() -> bool
{
    std::cout << "XFTPServerCMD::init()" << std::endl;
    /// ����socket bufferevent
    /// base socket

    bufferevent* bev = bufferevent_socket_new(base_, sock_, BEV_OPT_CLOSE_ON_FREE);
    bufferevent_setcb(bev, read_cb, 0, event_cb, this);
    bufferevent_enable(bev, EV_READ | EV_WRITE);

    /// ��ӳ�ʱ
    timeval rt_ = { 10, 0 };
    bufferevent_set_timeouts(bev, &rt_, 0);

    return true;
}
