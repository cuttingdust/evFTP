#include "XFTPStor.h"

#include <iostream>
#include <ostream>

#include <event2/bufferevent.h>
#include <event2/event.h>

auto XFTPStor::parse(const std::string &type, const std::string &msg) -> void
{
    /// �ļ���
    int         pos      = msg.rfind(" ") + 1;
    std::string filename = msg.substr(pos, msg.size() - pos - 2);
    std::string path     = cmdTask_->rootDir_;
    path += cmdTask_->curDir_;
    if (path[path.size() - 1] != '/')
    {
        path += "/";
    }
    path += filename;

    if (auto fp = fopen(path.c_str(), "wb"))
    {
        /// ��������ͨ��
        connectPORT();
        this->fp_ = fp;

        /// ���Ϳ�ʼ�����ļ���ָ��
        resCMD("125 File OK\r\n");

        /// ������ȡ�¼�
        bufferevent_trigger(bev_, EV_READ, 0);
    }
    else
    {
        resCMD("450 file open failed!\r\n");
    }
}

auto XFTPStor::read(struct bufferevent *bev) -> void
{
    if (!fp_)
    {
        return;
    }
    for (;;)
    {
        int len = bufferevent_read(bev, buf, sizeof(buf));
        if (len <= 0)
        {
            return;
        }
        int size = fwrite(buf, 1, len, fp_);
        std::cout << "<" << len << ":" << size << ">" << std::flush;
    }
}

auto XFTPStor::event(struct bufferevent *bev, short what) -> void
{
    /// ����Է�����ϵ������߻��������п����ղ���BEV_EVENT_EOF����
    if (what & (BEV_EVENT_EOF | BEV_EVENT_ERROR | BEV_EVENT_TIMEOUT))
    {
        std::cout << "XFTPStor BEV_EVENT_EOF | BEV_EVENT_ERROR |BEV_EVENT_TIMEOUT" << std::endl;
        close();
        resCMD("226 Transfer complete\r\n");
    }
    else if (what & BEV_EVENT_CONNECTED)
    {
        std::cout << "XFTPStor BEV_EVENT_CONNECTED" << std::endl;
    }
}
