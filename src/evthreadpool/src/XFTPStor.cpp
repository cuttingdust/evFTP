#include "XFTPStor.h"

#include <iostream>
#include <ostream>

#include <event2/bufferevent.h>
#include <event2/event.h>

auto XFTPStor::parse(const std::string &type, const std::string &msg) -> void
{
    /// 文件名
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
        /// 连接数据通道
        connectPORT();
        this->fp_ = fp;

        /// 发送开始接收文件的指令
        resCMD("125 File OK\r\n");

        /// 触发读取事件
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
    /// 如果对方网络断掉，或者机器死机有可能收不到BEV_EVENT_EOF数据
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
