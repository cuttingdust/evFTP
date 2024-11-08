#include "XFTPList.h"

#include <event2/bufferevent.h>
#include <io.h>

#include <iostream>


XFTPList::XFTPList()
{
}

XFTPList::~XFTPList()
{
}

auto XFTPList::parse(const std::string &type, const std::string &msg) -> void
{
    std::string resmsg = "";
    if (type == "PWD")
    {
        /// 257 "/" is current directory.
        resmsg = "257 \"";
        resmsg += cmdTask_->curDir_;
        resmsg += "\" is current dir.\r\n";

        resCMD(resmsg);
    }
    else if (type == "LIST")
    {
        /// 1连接数据通道
        /// 2 150
        /// 3 发送目录数据通道
        /// 4 发送完成226 5 关闭连接
        /// 命令通道回复消息 使用数据通道发送目录
        /// -rwxrwxrwx 1 root group 64463 Mar 14 09:53 101.jpg\r\n

        /// 1 连接数据通道
        connectPORT();

        /// 2 1502 150
        resCMD("150 Here comes the directory listing.\r\n");

        /// 3 数据通道发送
        // std::string listdata = "-rwxrwxrwx 1 root group 64463 Mar 14 09:53 101.jpg\r\n";
        const std::string listdata = getListData(cmdTask_->rootDir_ + cmdTask_->curDir_);

        send(listdata);
    }
}

auto XFTPList::write(struct bufferevent *bev) -> void
{
    /// 4 226 Transfer complete发送完成
    resCMD("226 Transfer complete\r\n");
    /// 5 关闭连接
    close();
}

auto XFTPList::event(struct bufferevent *bev, short what) -> void
{
    /// 如果对方网络断掉，或者机器死机有可能收不到BEV_EVENT_EOF数据
    if (what & (BEV_EVENT_EOF | BEV_EVENT_ERROR | BEV_EVENT_TIMEOUT))
    {
        std::cout << "BEV_EVENT_EOF | BEV_EVENT_ERROR |BEV_EVENT_TIMEOUT" << std::endl;
        close();
    }
    else if (what & BEV_EVENT_CONNECTED)
    {
        std::cout << "XFtpLIST BEV_EVENT_CONNECTED" << std::endl;
    }
}

std::string XFTPList::getListData(const std::string &path)
{
    /// -rwxrwxrwx 1 root group 64463 Mar 14 09:53 101.jpg\r\n
    std::string data = "";
    /// 存储文件信息
    _finddata_t file;

    /// 目录上下文
    const std::string filePaths = path + "/*.*";
    intptr_t          dir       = _findfirst(filePaths.c_str(), &file);
    if (dir < 0)
    {
        return data;
    }

    do
    {
        std::string tmp = "";
        /// 是否是目录 去掉 .和..
        if (file.attrib & _A_SUBDIR)
        {
            if (strcmp(file.name, ".") == 0 || strcmp(file.name, "..") == 0)
            {
                continue;
            }
            tmp += "drwxrwxrwx 1 root group ";
        }
        else
        {
            tmp += "-rwxrwxrwx 1 root group ";
        }

        /// 文件大小
        char buf[1024];
        sprintf(buf, "%u", file.size);
        tmp += buf;

        /// 日期时间
        strftime(buf, sizeof(buf) - 1, "%b %d %H:%M", localtime(&file.time_write));
        tmp += " ";
        tmp += buf;
        tmp += " ";
        tmp += file.name;
        tmp += "\r\n";
        data += tmp;

        std::cout << __func__ << " " << tmp << std::endl;
    }
    while (_findnext(dir, &file) == 0);

    return data;
}
