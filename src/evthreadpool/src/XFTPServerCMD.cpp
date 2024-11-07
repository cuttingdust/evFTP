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

/// ע���������� ����Ҫ�����̰߳�ȫ ����ʱ��δ�ַ����߳�
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

    /// �Ѿ�ע����Ƿ񸲸� �����ǣ���ʾ����
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
    /// ����socket bufferevent
    /// base socket

    bufferevent* bev = bufferevent_socket_new(base_, sock_, BEV_OPT_CLOSE_ON_FREE);
    this->bev_       = bev;
    this->setCallback(bev);

    /// ��ӳ�ʱ
    timeval rt_ = { 60, 0 };
    bufferevent_set_timeouts(bev, &rt_, 0);

    std::string msg = "220 Welcome to libevent XFtpServer\r\n";
    bufferevent_write(bev, msg.c_str(), msg.size());

    return true;
}

/// ���߳�XThread  event�¼��ַ�
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
        /// �ַ����������
        /// ���������� USER anonymous
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
            t->cmdTask_ = this; /// ��������ظ������Ŀ¼
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

    /// ����Է�����ϵ������߻��������п����ղ���BEV_EVENT_EOF����
    if (what & (BEV_EVENT_EOF | BEV_EVENT_ERROR | BEV_EVENT_TIMEOUT))
    {
        std::cout << "BEV_EVENT_EOF | BEV_EVENT_ERROR | BEV_EVENT_TIMEOUT" << std::endl;
        bufferevent_free(bev);
        delete this;
    }
}
