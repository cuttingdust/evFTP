#include "XMsgServerEvent.h"

#include "XMsgCom.pb.h"

class XMsgServerEvent::PImpl
{
public:
    PImpl(XMsgServerEvent *owenr);
    ~PImpl() = default;

public:
    XMsgServerEvent                                           *owenr_ = nullptr;
    static std::map<XMsg::MsgType, XMsgServerEvent::MsgCBFunc> calls_;
};
std::map<XMsg::MsgType, XMsgServerEvent::MsgCBFunc> XMsgServerEvent::PImpl::calls_;

XMsgServerEvent::PImpl::PImpl(XMsgServerEvent *owenr) : owenr_(owenr)
{
}


XMsgServerEvent::XMsgServerEvent()
{
    impl_ = std::make_shared<PImpl>(this);
}

XMsgServerEvent::~XMsgServerEvent() = default;

void XMsgServerEvent::initEvent()
{
    regCB(XMsg::MT_LOGIN_REQ, &XMsgServerEvent::loginReq);
}

void XMsgServerEvent::loginReq(const char *data, int size)
{
    std::cout << "LoginReq " << size << std::endl;

    /// 解包 反序列化
    XMsg::XLoginReq req;
    req.ParseFromArray(data, size);
    std::cout << "Recv username = " << req.username() << std::endl;
    std::cout << "Recv password = " << req.password() << std::endl;

    /// 返回消息
    XMsg::XLoginRes res;
    res.set_restype(XMsg::XLoginRes::XRT_OK);
    std::string token = req.username();
    token += " sign.";
    res.set_token(token);
    sendMsg(XMsg::MT_LOGIN_RES, &res);
}

void XMsgServerEvent::regCB(XMsg::MsgType type, const MsgCBFunc &func)
{
    XMsgServerEvent::PImpl::calls_[type] = func;
}

void XMsgServerEvent::callFunc(XMsg::MsgType type, const char *data, int size)
{
    const auto it = XMsgServerEvent::PImpl::calls_.find(type);
    if (it != XMsgServerEvent::PImpl::calls_.end())
    {
        XMsgServerEvent::MsgCBFunc func = it->second;
        (this->*func)(data, size);
    }
}
