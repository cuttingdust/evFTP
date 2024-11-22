#include "XMsgClientEvent.h"

#include "XMsgType.pb.h"
#include "XMsgCom.pb.h"

#include <map>


class XMsgClientEvent::PImpl
{
public:
    PImpl(XMsgClientEvent *owenr);
    ~PImpl() = default;

public:
    XMsgClientEvent                                           *owenr_ = nullptr;
    static std::map<XMsg::MsgType, XMsgClientEvent::MsgCBFunc> calls_;
};
std::map<XMsg::MsgType, XMsgClientEvent::MsgCBFunc> XMsgClientEvent::PImpl::calls_;

XMsgClientEvent::PImpl::PImpl(XMsgClientEvent *owenr) : owenr_(owenr)
{
}


XMsgClientEvent::XMsgClientEvent()
{
    impl_ = std::make_shared<PImpl>(this);
}

XMsgClientEvent::~XMsgClientEvent() = default;

void XMsgClientEvent::initEvent()
{
    regCB(XMsg::MsgType::MT_LOGIN_RES, &XMsgClientEvent::loginRes);
}

void XMsgClientEvent::loginRes(const char *data, int size)
{
    std::cout << "LoginRes " << size << std::endl;
    /// 解封 反序列化
    XMsg::XLoginRes res;
    res.ParseFromArray(data, size);
    std::cout << "recv server restype=" << res.restype() << ", recv server token =" << res.token() << std::endl;

    XMsg::XLoginReq req;
    char            buf[1024] = { 0 };
    static int      count     = 0;
    count++;
    sprintf(buf, "root_%d", count);
    req.set_username(buf);
    req.set_password("123456");
    sendMsg(XMsg::MT_LOGIN_REQ, &req);
}

void XMsgClientEvent::regCB(XMsg::MsgType type, const MsgCBFunc &func)
{
    XMsgClientEvent::PImpl::calls_[type] = func;
}

void XMsgClientEvent::callFunc(XMsg::MsgType type, const char *data, int size)
{
    const auto it = XMsgClientEvent::PImpl::calls_.find(type);
    if (it != XMsgClientEvent::PImpl::calls_.end())
    {
        XMsgClientEvent::MsgCBFunc func = it->second;
        (this->*func)(data, size);
    }
}
