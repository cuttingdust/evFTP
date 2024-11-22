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
    XMessage            head_; /// 消息头
    XMessage            msg_;  /// 消息内容
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
        std::cerr << "XMsgEvent::recvMsg() failed：bev not set" << std::endl;
        return false;
    }

    //////////////////////////////解包/////////////////////////////

    /// 接收消息头
    if (!impl_->head_.size)
    {
        /// 1 消息头大小
        int len = bufferevent_read(impl_->bev_, &impl_->head_.size, sizeof(impl_->head_.size));
        if (len <= 0 || impl_->head_.size <= 0)
        {
            return false;
        }

        /// 分配消息头空间 读取消息头（鉴权，消息大小）
        if (!impl_->head_.alloc(impl_->head_.size))
        {
            std::cerr << "head_.alloc failed!" << std::endl;
            return false;
        }
    }

    /// 2 开始接收消息头（鉴权，消息大小）
    if (!impl_->head_.recved())
    {
        int len = bufferevent_read(impl_->bev_,
                                   impl_->head_.data + impl_->head_.recvSize, /// 第二次进来 从上次的位置开始读
                                   impl_->head_.size - impl_->head_.recvSize);
        if (len <= 0)
        {
            return true;
        }
        impl_->head_.recvSize += len;
        if (!impl_->head_.recved())
            return true;

        /// 完整的头部数据接收完成
        /// 反序列化
        XMsg::XMsgHead pb_head;
        if (!pb_head.ParseFromArray(impl_->head_.data, impl_->head_.size))
        {
            std::cerr << "pb_head.ParseFromArray failed!" << std::endl;
            return false;
        }

        /// 鉴权
        /// 消息内容大小
        /// 分配消息内容空间
        if (!impl_->msg_.alloc(pb_head.msgsize()))
        {
            std::cerr << "msg_.alloc failed!" << std::endl;
            return false;
        }
    }

    /// 3 开始接收消息内容
    if (!impl_->msg_.recved())
    {
        int len = bufferevent_read(impl_->bev_,
                                   impl_->msg_.data + impl_->msg_.recvSize, /// 第二次进来 从上次的位置开始读
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

    ////////////////////////封包////////////////////////

    /// 消息内容序列化
    std::string msgStr  = msg->SerializeAsString();
    int         msgSize = msgStr.size();
    head.set_msgsize(msgSize);

    /// 消息头序列化
    std::string headStr  = head.SerializeAsString();
    int         headSize = headStr.size();

    /// 1 发送消息头大小 4字节 暂时不考虑字节序问题
    bufferevent_write(impl_->bev_, &headSize, sizeof(headSize));

    /// 2 发送消息头（pb序列化） XMsgHead （设置消息内容的大小）
    bufferevent_write(impl_->bev_, headStr.data(), headStr.size());

    /// 3 发送消息内容 （pb序列化） 业务proto
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
