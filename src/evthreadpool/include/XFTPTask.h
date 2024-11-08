/**
 * @file   XFTPTask.h
 * @brief  
 *
 * Detailed description if necessary.
 *
 * @author 31667
 * @date   2024-11-07
 */

#ifndef XFTPTASK_H
#define XFTPTASK_H

#include "XTask.h"

#include <string>

class XFTPTask : public XTask
{
public:
    virtual auto read(struct bufferevent *bev) -> void{};
    virtual auto write(struct bufferevent *bev) -> void{};
    virtual auto event(struct bufferevent *bev, short what) -> void{};
    auto         setCallback(struct bufferevent *bev) -> void;

    /// \brief 解析协议
    /// \param type
    /// \param msg
    virtual auto parse(const std::string &type, const std::string &msg) -> void{};

    auto init() -> bool override
    {
        return true;
    }

protected:
    static auto readCB(bufferevent *bev, void *arg) -> void;
    static auto writeCB(bufferevent *bev, void *arg) -> void;
    static auto eventCB(struct bufferevent *bev, short what, void *arg) -> void;


public:
    /// \brief 回复cmd消息
    /// \param msg
    auto resCMD(std::string msg) -> void;

    /// \brief 连接数据通道
    auto connectPORT() -> void;

    /// \brief 关闭数据通道
    auto close() -> void;

    /// \brief 发送数据
    /// \param data
    auto send(const std::string &data) -> void;

    /// \brief 发送数据
    /// \param data
    /// \param dataSize
    auto send(const char *data, int dataSize) -> void;


public:
    std::string curDir_  = "/";
    std::string rootDir_ = ".";
    XFTPTask   *cmdTask_ = nullptr;

    std::string ip_   = "";
    int         port_ = 0;

protected:
    struct bufferevent *bev_ = 0;
};

#endif // XFTPTASK_H
