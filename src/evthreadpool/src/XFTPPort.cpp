#include "XFTPPort.h"

#include <iostream>
#include <vector>

XFTPPort::XFTPPort()
{
}

XFTPPort::~XFTPPort()
{
}

auto XFTPPort::parse(const std::string &type, const std::string &msg) -> void
{
    /// PORT 127,0,0,1,238,141\r\n
    /// PORT n1,n2,n3,n4,n5,n6\r\n
    /// port = n5*256 + n6

    /// 只获取ip和端口，不连接
    /// 取出ip
    std::vector<std::string> vals;
    std::string              tmp;
    size_t                   iSize = msg.size();
    for (int i = 5; i < iSize; ++i)
    {
        if (msg[i] == ',' || msg[i] == '\r')
        {
            vals.emplace_back(tmp);
            tmp.clear();
            continue;
        }
        tmp += msg[i];
    }
    if (vals.size() != 6)
    {
        /// PORT 格式错误
        resCMD("501 Syntax error in parameters or arguments.");
        return;
    }
    ip_ = vals[0] + "." + vals[1] + "." + vals[2] + "." + vals[3];

    ///port = n5*256 + n6 
    port_ = atoi(vals[4].c_str()) * 256 + atoi(vals[5].c_str());
    std::cout << "PORT ip is " << ip_ << std::endl;
    std::cout << "PORT port is " << port_ << std::endl;

    resCMD("200 PORT command successful.\r\n");
}
