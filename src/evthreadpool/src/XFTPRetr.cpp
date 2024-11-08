#include "XFTPRetr.h"

#include <iostream>

#include <event2/bufferevent.h>
#include <event2/event.h>

XFTPRetr::XFTPRetr()
{
}

XFTPRetr::~XFTPRetr()
{
}

auto XFTPRetr::parse(const std::string &type, const std::string &msg) -> void
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
    if (auto fp = fopen(path.c_str(), "rb"))
    {
        /// 连接数据通道
        connectPORT();
        this->fp_ = fp;

        /// 发送开始下载文件的指令
        resCMD("150 File OK\r\n");
        /// 触发写入事件
        bufferevent_trigger(bev_, EV_WRITE, 0);
    }
    else
    {
        resCMD("550 File not found\r\n");
    }
}

auto XFTPRetr::write(struct bufferevent *bev) -> void
{
    if (!fp_)
    {
        return;
    }
    int len = fread(buf, 1, sizeof(buf), fp_);
    if (len <= 0)
    {
        resCMD("226 Transfer complete\r\n");
        close();
        return;
    }

    std::cout << "[" << len << "]" << std::flush;
    send(buf, len);
}

auto XFTPRetr::event(struct bufferevent *bev, short what) -> void
{
    /// 如果对方网络断掉，或者机器死机有可能收不到BEV_EVENT_EOF数据
    if (what & (BEV_EVENT_EOF | BEV_EVENT_ERROR | BEV_EVENT_TIMEOUT))
    {
        std::cout << "BEV_EVENT_EOF | BEV_EVENT_ERROR |BEV_EVENT_TIMEOUT" << std::endl;
        close();
    }
    else if (what & BEV_EVENT_CONNECTED)
    {
        std::cout << "XFTPRetr BEV_EVENT_CONNECTED" << std::endl;
    }
}
