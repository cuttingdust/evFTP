#include "XMsgClient.h"

#include "XMsgCom.pb.h"
#include "XMsgEvent.h"


#include <event2/bufferevent.h>
#include <event2/event.h>

#include <thread>
#include <iostream>

using namespace std::chrono_literals;

/// ��ȡ����
static void read_cb(struct bufferevent *be, void *arg)
{
    std::cout << "[CR]:" << std::flush;
    auto ev = static_cast<XMsgEvent *>(arg);
    if (!ev->recvMsg())
    {
        delete ev;
        bufferevent_free(be);
        return;
    }
    auto msg = ev->getMsg();
    if (!msg)
    {
        return;
    }
    XMsg::XLoginRes res;
    res.ParseFromArray(msg->data, msg->size);
    std::cout << "recv server restype=" << res.restype() << ", recv server token =" << res.token() << std::endl;

    XMsg::XLoginReq req;
    char            buf[1024] = { 0 };
    static int      count     = 0;
    count++;
    sprintf(buf, "root_%d", count);
    req.set_username(buf);
    req.set_password("123456");
    ev->sendMsg(XMsg::MT_LOGIN_REQ, &req);
    ev->clear(); /// ������ʼ������һ����Ϣ

    // char data[1024] = { 0 };
    // /// ��ȡ���뻺������
    // int len = bufferevent_read(be, data, sizeof(data) - 1);
    // std::cout << "[" << data << "]" << std::endl;
    // if (len <= 0)
    //     return;
    // if (strstr(data, "quit") != nullptr)
    // {
    //     std::cout << "quit" << std::endl;
    //     /// �˳����ر�socket BEV_OPT_CLOSE_ON_FREE
    //     bufferevent_free(be);
    // }

    // /// �������� д�뵽�������
    // bufferevent_write(be, "hello", 6);

    /// ֻ�ǲ����ã��ص��в����������Ĵ���Ӱ��������Ϣ����
    std::this_thread::sleep_for(1000ms);
}

/// ���󣬳�ʱ �����ӶϿ�����룩
static void event_cb(struct bufferevent *be, short events, void *arg)
{
    std::cout << "[CE]:" << std::flush;
    auto ev = static_cast<XMsgEvent *>(arg);

    /// ��ȡ��ʱʱ�䷢�������ݶ�ȡֹͣ
    if (events & (BEV_EVENT_TIMEOUT | BEV_EVENT_READING))
    {
        std::cout << "BEV_EVENT_READING BEV_EVENT_TIMEOUT" << std::endl;
        bufferevent_free(be);
    }
    else if (events & (BEV_EVENT_ERROR | BEV_EVENT_EOF))
    {
        std::cout << "BEV_EVENT_ERROR  | BEV_EVENT_EOF" << std::endl;
        bufferevent_free(be);
    }
    else if (events & BEV_EVENT_CONNECTED)
    {
        std::cout << "BEV_EVENT_CONNECTED" << std::endl;
        // bufferevent_write(be, "start hello", 13);
        XMsg::XLoginReq req;
        req.set_username("root");
        req.set_password("123456");
        ev->sendMsg(XMsg::MT_LOGIN_REQ, &req);
    }
    else
    {
        std::cout << "OTHERS" << std::endl;
    }
}

class XMsgClient::PImpl
{
public:
    PImpl(XMsgClient *owenr);
    ~PImpl() = default;

public:
    void threadFunc();

public:
    XMsgClient *owenr_       = nullptr;
    std::string server_ip_   = "";
    int         server_port_ = 0;
};

XMsgClient::PImpl::PImpl(XMsgClient *owenr) : owenr_(owenr)
{
}

void XMsgClient::PImpl::threadFunc()
{
    std::cout << "XMsgClient::PImpl::threadFunc start" << std::endl;

    if (server_port_ < 0)
    {
        std::cerr << "server port is invalid!" << std::endl;
        return;
    }

    /// �ȴ����������
    std::this_thread::sleep_for(200ms);

    /// ����libevent��������
    event_base *base = event_base_new();
    if (base)
    {
        std::cout << "event_base_new success!" << std::endl;
    }

    /// ���ӷ�����
    bufferevent *bev = bufferevent_socket_new(base, -1, BEV_OPT_CLOSE_ON_FREE);

    sockaddr_in sin;
    memset(&sin, 0, sizeof(sin));
    sin.sin_family = AF_INET;
    sin.sin_port   = htons(server_port_); // NOLINT(clang-diagnostic-implicit-int-conversion)
    if (server_ip_.empty())
    {
        server_ip_ = "127.0.0.1";
    }
    evutil_inet_pton(AF_INET, server_ip_.c_str(), &sin.sin_addr.s_addr);

    bufferevent_enable(bev, EV_WRITE | EV_READ);

    /// ��ʱ�趨 �룬΢�루1/1000000�룩  ����ʱ ��д��ʱ
    timeval t1 = { 30, 0 };
    bufferevent_set_timeouts(bev, &t1, 0);

    auto ev = new XMsgEvent;
    ev->setBev(bev);

    ///���ûص�����
    bufferevent_setcb(bev, read_cb, nullptr, event_cb, ev);


    int re = bufferevent_socket_connect(bev, (sockaddr *)&sin, sizeof(sin));
    if (re != 0)
    {
        std::cerr << "bufferevent_socket_connect error" << std::endl;
        return;
    }

    /// �¼���ѭ�� ����¼��Ƿ��ͣ� �ַ��¼����ص�����
    /// ���û���¼�ע�����˳�
    event_base_dispatch(base);
    event_base_free(base);

    std::cout << "XMsgClient::PImpl::threadFunc end" << std::endl;
}


XMsgClient::XMsgClient()
{
    impl_ = std::make_shared<PImpl>(this);
}

XMsgClient::~XMsgClient() = default;

void XMsgClient::start()
{
    std::cout << "XMsgClient::start" << std::endl;
    std::thread(&XMsgClient::PImpl::threadFunc, impl_).detach();
}

void XMsgClient::setServerIp(const std::string &ip)
{
    impl_->server_ip_ = ip;
    std::cout << "XMsgClient::setServerIp: " << impl_->server_ip_ << std::endl;
}

void XMsgClient::setServerPort(int port)
{
    impl_->server_port_ = port;
    std::cout << "XMsgClient::setServerPort: " << impl_->server_port_ << std::endl;
}
