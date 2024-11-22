#include "XMsgEvent.h"
#include "XMsgCom.pb.h"

#include <event2/bufferevent.h>

class XMsgEvent::PImpl
{
public:
    PImpl(XMsgEvent *owenr);
    ~PImpl() = default;

public:
    XMsgEvent          *owenr_ = nullptr;
    struct bufferevent *bev_   = nullptr;
    XMessage            head_; /// ��Ϣͷ
    XMessage            msg_;  /// ��Ϣ����
};

XMsgEvent::PImpl::PImpl(XMsgEvent *owenr) : owenr_(owenr)
{
}


XMsgEvent::XMsgEvent()
{
    impl_ = std::make_shared<PImpl>(this);
}

XMsgEvent::~XMsgEvent() = default;

bool XMsgEvent::recvMsg()
{
    if (!impl_->bev_)
    {
        std::cerr << "XMsgEvent::recvMsg() failed��bev not set" << std::endl;
        return false;
    }

    //////////////////////////////���/////////////////////////////

    /// ������Ϣͷ
    if (!impl_->head_.size)
    {
        /// 1 ��Ϣͷ��С
        int len = bufferevent_read(impl_->bev_, &impl_->head_.size, sizeof(impl_->head_.size));
        if (len <= 0 || impl_->head_.size <= 0)
        {
            return false;
        }

        /// ������Ϣͷ�ռ� ��ȡ��Ϣͷ����Ȩ����Ϣ��С��
        if (!impl_->head_.alloc(impl_->head_.size))
        {
            std::cerr << "head_.alloc failed!" << std::endl;
            return false;
        }
    }

    /// 2 ��ʼ������Ϣͷ����Ȩ����Ϣ��С��
    if (!impl_->head_.recved())
    {
        int len = bufferevent_read(impl_->bev_,
                                   impl_->head_.data + impl_->head_.recvSize, /// �ڶ��ν��� ���ϴε�λ�ÿ�ʼ��
                                   impl_->head_.size - impl_->head_.recvSize);
        if (len <= 0)
        {
            return true;
        }
        impl_->head_.recvSize += len;
        if (!impl_->head_.recved())
            return true;

        /// ������ͷ�����ݽ������
        /// �����л�
        XMsg::XMsgHead pb_head;
        if (!pb_head.ParseFromArray(impl_->head_.data, impl_->head_.size))
        {
            std::cerr << "pb_head.ParseFromArray failed!" << std::endl;
            return false;
        }

        /// ��Ȩ
        /// ��Ϣ���ݴ�С
        /// ������Ϣ���ݿռ�
        if (!impl_->msg_.alloc(pb_head.msgsize()))
        {
            std::cerr << "msg_.alloc failed!" << std::endl;
            return false;
        }
    }

    /// 3 ��ʼ������Ϣ����
    if (!impl_->msg_.recved())
    {
        int len = bufferevent_read(impl_->bev_,
                                   impl_->msg_.data + impl_->msg_.recvSize, /// �ڶ��ν��� ���ϴε�λ�ÿ�ʼ��
                                   impl_->msg_.size - impl_->msg_.recvSize);
        if (len <= 0)
        {
            return true;
        }
        impl_->msg_.recvSize += len;
        if (!impl_->msg_.recved())
            return true;
    }

    if (impl_->msg_.recved())
    {
        std::cout << "msg_.recved()" << std::endl;
    }

    return true;
}

auto XMsgEvent::getMsg() const -> XMessage *
{
    return impl_->msg_.recved() ? &impl_->msg_ : nullptr;
}

void XMsgEvent::sendMsg(const XMsg::MsgType &msgType, const google::protobuf::Message *msg)
{
    if (!impl_->bev_ || !msg)
        return;

    XMsg::XMsgHead head;
    head.set_msgtype(msgType);

    ////////////////////////���////////////////////////

    /// ��Ϣ�������л�
    std::string msgStr  = msg->SerializeAsString();
    int         msgSize = msgStr.size();
    head.set_msgsize(msgSize);

    /// ��Ϣͷ���л�
    std::string headStr  = head.SerializeAsString();
    int         headSize = headStr.size();

    /// 1 ������Ϣͷ��С 4�ֽ� ��ʱ�������ֽ�������
    bufferevent_write(impl_->bev_, &headSize, sizeof(headSize));

    /// 2 ������Ϣͷ��pb���л��� XMsgHead ��������Ϣ���ݵĴ�С��
    bufferevent_write(impl_->bev_, headStr.data(), headStr.size());

    /// 3 ������Ϣ���� ��pb���л��� ҵ��proto
    bufferevent_write(impl_->bev_, msgStr.data(), msgStr.size());
}

void XMsgEvent::clear()
{
    impl_->head_.clear();
    impl_->msg_.clear();
}

void XMsgEvent::setBev(struct bufferevent *bev)
{
    impl_->bev_ = bev;
}
