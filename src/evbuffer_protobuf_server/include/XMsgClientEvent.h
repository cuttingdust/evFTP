/**
 * @file   XMsgClientEvent.h
 * @brief  
 *
 * Detailed description if necessary.
 *
 * @author 31667
 * @date   2024-11-22
 */

#ifndef XMSGCLIENTEVENT_H
#define XMSGCLIENTEVENT_H

#include "XMsgEvent.h"

class XMsgClientEvent final : public XMsgEvent
{
public:
    XMsgClientEvent();
    ~XMsgClientEvent() override;

public:
    /// \brief ��ʼ���ص�����
    static void initEvent();

    /// \brief ���ܵ�½������Ϣ
    /// \param data ��Ϣ����
    /// \param size ��Ϣ��С
    void loginRes(const char *data, int size);

    ///��Ա����ָ������
    typedef void (XMsgClientEvent::*MsgCBFunc)(const char *data, int size);

    /// \brief ע����Ϣ�ص����� ֻ��Ҫע��һ�Σ����ھ�̬map
    /// \param type ��Ϣ����
    /// \param func ��Ϣ�ص�����
    static void regCB(XMsg::MsgType type, const MsgCBFunc &func);

    /// \brief ͨ�����ͺͳ�Ա����ָ����ú���
    /// \param type ��Ϣ����
    /// \param data ��Ϣ����
    /// \param size ��Ϣ��С
    void callFunc(XMsg::MsgType type, const char *data, int size);

private:
    class PImpl;
    std::shared_ptr<PImpl> impl_;
};


#endif // XMSGCLIENTEVENT_H
