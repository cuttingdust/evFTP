/**
 * @file   XFTPServerCMD.h
 * @brief  
 *
 * Detailed description if necessary.
 *
 * @author 31667
 * @date   2024-11-06
 */

#ifndef XFTPSERVERCMD_H
#define XFTPSERVERCMD_H
#include "XFTPTask.h"

#include <map>

class XFTPServerCMD : public XFTPTask
{
public:
    XFTPServerCMD();
    virtual ~XFTPServerCMD();

public:
    /// \brief 注册任务
    /// \param cmd
    /// \param call
    auto reg(const std::string &cmd, XFTPTask *call) -> void;

public:
    auto init() -> bool override;
    auto read(struct bufferevent *bev) -> void override;
    auto event(struct bufferevent *bev, short what) -> void override;

private:
    /// 注册的处理对象
    std::map<std::string, XFTPTask *> calls_;

    std::map<XFTPTask *, int> calls_del_;
};

#endif // XFTPSERVERCMD_H
