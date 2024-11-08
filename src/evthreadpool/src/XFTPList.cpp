#include "XFTPList.h"

#include <event2/bufferevent.h>
// #include <io.h>

#include <iostream>
#include <filesystem>
#include <sstream>

namespace fs = std::filesystem;

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
    else if (type == "CWD") /// 切换目录
    {
        /// 取出命令中的路径
        /// CWD /test/
        int pos = msg.rfind(" ") + 1;
        /// 去掉结尾的\r\n
        const std::string path = msg.substr(pos, msg.size() - pos - 2);
        if (path[0] == '/') /// 局对路径
        {
            cmdTask_->curDir_ = path;
        }
        else
        {
            if (cmdTask_->curDir_[cmdTask_->curDir_.size() - 1] != '/')
            {
                cmdTask_->curDir_ += "/";
            }
            cmdTask_->curDir_ += path + "/";
        }

        ///  /test/
        resCMD("250 Directory succes chanaged.\r\n");
    }
    else if (type == "CDUP") /// 回到上层目录
    {
        ///  /Debug/test_ser.A3C61E95.tlog /Debug   /Debug/
        std::string path = cmdTask_->curDir_;
        /// 统一去掉结尾的 /
        ///  /Debug/test_ser.A3C61E95.tlog /Debug
        if (path[path.size() - 1] == '/')
        {
            path = path.substr(0, path.size() - 1);
        }
        int pos = path.rfind("/");
        if (pos == -1)
        {
            resCMD("250 Directory succes chanaged.\r\n");
            return;
        }
        cmdTask_->curDir_ = path.substr(0, pos + 1);
        resCMD("250 Directory succes chanaged.\r\n");
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

std::string format_file_time(const fs::file_time_type &file_time)
{
    /// 将 file_time_type 转换为系统时间
    auto sctp = std::chrono::time_point_cast<std::chrono::system_clock::duration>(
            file_time - fs::file_time_type::clock::now() + std::chrono::system_clock::now());

    auto time_t = std::chrono::system_clock::to_time_t(sctp);

    /// 使用 std::stringstream 格式化时间
    std::stringstream ss;
    ss << std::put_time(std::localtime(&time_t), "%b %d %H:%M");
    return ss.str();
}


auto XFTPList::getListData(const std::string &path) -> std::string
{
    /// -rwxrwxrwx 1 root group 64463 Mar 14 09:53 101.jpg\r\n
    std::string data = "";
    try
    {
        for (const auto &entry : fs::directory_iterator(path))
        {
            std::string tmp         = "";
            std::string permissions = entry.is_directory() ? "drwxrwxrwx" : "-rwxrwxrwx";
            tmp += permissions + " 1 root group ";

            tmp += std::to_string(entry.file_size()) + " ";

            /// 日期时间，这里简化使用系统默认的本地化时间格式，可以自定义时间格式或者获取特定的时区信息
            tmp += format_file_time(fs::last_write_time(entry)) + " ";

            /// 文件名
            tmp += entry.path().filename().string() + "\r\n";

            std::cout << tmp;
            data += tmp;
        }
    }
    catch (const fs::filesystem_error &e)
    {
        std::cerr << "Error while listing directory: " << e.what() << std::endl;
        return "";
    }
    catch (...)
    {
        std::cerr << "Unknown error occurred." << std::endl;
        return "";
    }
    return data;
}

// auto XFTPList::getListData(const std::string &path) -> std::string
// {
//     /// -rwxrwxrwx 1 root group 64463 Mar 14 09:53 101.jpg\r\n
//     std::string data = "";
//     /// 存储文件信息
//     _finddata_t file;
//
//     /// 目录上下文
//     const std::string filePaths = path + "/*.*";
//     intptr_t          dir       = _findfirst(filePaths.c_str(), &file);
//     if (dir < 0)
//     {
//         return data;
//     }
//
//     do
//     {
//         std::string tmp = "";
//         /// 是否是目录 去掉 .和..
//         if (file.attrib & _A_SUBDIR)
//         {
//             if (strcmp(file.name, ".") == 0 || strcmp(file.name, "..") == 0)
//             {
//                 continue;
//             }
//             tmp += "drwxrwxrwx 1 root group ";
//         }
//         else
//         {
//             tmp += "-rwxrwxrwx 1 root group ";
//         }
//
//         /// 文件大小
//         char buf[1024];
//         sprintf(buf, "%u", file.size);
//         tmp += buf;
//
//         /// 日期时间
//         strftime(buf, sizeof(buf) - 1, "%b %d %H:%M", localtime(&file.time_write));
//         tmp += " ";
//         tmp += buf;
//         tmp += " ";
//         tmp += file.name;
//         tmp += "\r\n";
//         data += tmp;
//
//         // std::cout << __func__ << " " << tmp;
//     }
//     while (_findnext(dir, &file) == 0);
//
//     return data;
// }
