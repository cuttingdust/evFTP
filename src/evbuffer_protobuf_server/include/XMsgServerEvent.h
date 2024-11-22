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
    /// \brief ��ʼ���ص�����
    static void initEvent();

    /// \brief ���ܵ�¼��������Ϣ
    /// \param data ��Ϣ����
    /// \param size ��Ϣ��С
    void loginReq(const char *data, int size);

    ///��Ա����ָ������
    typedef void (XMsgServerEvent::*MsgCBFunc)(const char *data, int size);

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

#endif // XMSGSERVEREVENT_H
