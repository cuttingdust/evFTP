/**
 * @file   XMsgServerEvent.h
 * @brief  
 *
 * Detailed description if necessary.
 *
 * @author 31667
 * @date   2024-11-22
 */

#ifndef XMSGSERVEREVENT_H
#define XMSGSERVEREVENT_H

#include "XMsgEvent.h"
#include <memory>

class XMsgServerEvent final : public XMsgEvent
{
public:
    XMsgServerEvent();
    ~XMsgServerEvent() override;

public:
    /// \brief 初始化回调函数
    static void initEvent();

    /// \brief 接受登录的请求消息
    /// \param data 消息数据
    /// \param size 消息大小
    void loginReq(const char *data, int size);

    ///成员函数指针类型
    typedef void (XMsgServerEvent::*MsgCBFunc)(const char *data, int size);

    /// \brief 注册消息回调函数 只需要注册一次，存在静态map
    /// \param type 消息类型
    /// \param func 消息回调函数
    static void regCB(XMsg::MsgType type, const MsgCBFunc &func);

    /// \brief 通过类型和成员函数指针调用函数
    /// \param type 消息类型
    /// \param data 消息数据
    /// \param size 消息大小
    void callFunc(XMsg::MsgType type, const char *data, int size);

private:
    class PImpl;
    std::shared_ptr<PImpl> impl_;
};

#endif // XMSGSERVEREVENT_H
