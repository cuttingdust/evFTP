/**
 * @file   XMsgEvent.h
 * @brief  
 *
 * Detailed description if necessary.
 *
 * @author 31667
 * @date   2024-11-22
 */

#ifndef XMSGEVENT_H
#define XMSGEVENT_H
#include "XMsg.h"
#include "XMsgType.pb.h"

#include <memory>

class XMsgEvent
{
public:
    XMsgEvent();
    virtual ~XMsgEvent();

public:
    /// \brief �������ݰ���
    /// 1 ��ȷ���յ���Ϣ  (������Ϣ������)
    /// 2 ��Ϣ���ղ����� (�ȴ���һ�ν���)
    /// 3 ��Ϣ���ճ��� ���˳�����ռ䣩
    /// \return  1 2 ����true 3����false
    auto recvMsg() -> bool;

    /// \brief ��ȡ���յ������ݰ�����������ͷ����Ϣ��,
    /// �ɵ���������XMsg
    /// \return ���û�����������ݰ�������NULL
    auto getMsg() const -> XMessage *;

    /// \brief ������Ϣ ����ͷ�����Զ�������
    /// \param msgType  ��Ϣ����
    /// \param msg      ��Ϣ����
    auto sendMsg(const XMsg::MsgType &msgType, const google::protobuf::Message *msg) -> void;

    /// \brief ��������Ϣͷ����Ϣ���ݣ����ڽ�����һ����Ϣ
    auto clear() -> void;

    /// \brief
    /// \param bev
    auto setBev(struct bufferevent *bev) -> void;

private:
    class PImpl;
    std::shared_ptr<PImpl> impl_;
};


#endif // XMSGEVENT_H
