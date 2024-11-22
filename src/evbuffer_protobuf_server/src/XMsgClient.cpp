#include "XMsgClient.h"

#include "XMsgCom.pb.h"
#include "XMsgEvent.h"


#include <event2/bufferevent.h>
#include <event2/event.h>

#include <thread>
#include <iostream>

using namespace std::chrono_literals;

/// 读取数据
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
    ev->clear(); /// 清理，开始接收下一次消息

    // char data[1024] = { 0 };
    // /// 读取输入缓冲数据
    // int len = bufferevent_read(be, data, sizeof(data) - 1);
    // std::cout << "[" << data << "]" << std::endl;
    // if (len <= 0)
    //     return;
    // if (strstr(data, "quit") != nullptr)
    // {
    //     std::cout << "quit" << std::endl;
    //     /// 退出并关闭socket BEV_OPT_CLOSE_ON_FREE
    //     bufferevent_free(be);
    // }

    // /// 发送数据 写入到输出缓冲
    // bufferevent_write(be, "hello", 6);

    /// 只是测试用，回调中不能做高消耗处理，影响其他消息处理
    std::this_thread::sleep_for(1000ms);
}

/// 错误，超时 （连接断开会进入）
static void event_cb(struct bufferevent *be, short events, void *arg)
{
    std::cout << "[CE]:" << std::flush;
    auto ev = static_cast<XMsgEvent *>(arg);

    /// 读取超时时间发生后，数据读取停止
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

    /// 等待服务端启动
    std::this_thread::sleep_for(200ms);

    /// 创建libevent的上下文
    event_base *base = event_base_new();
    if (base)
    {
        std::cout << "event_base_new success!" << std::endl;
    }

    /// 连接服务器
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

    /// 超时设定 秒，微秒（1/1000000秒）  读超时 和写超时
    timeval t1 = { 30, 0 };
    bufferevent_set_timeouts(bev, &t1, 0);

    auto ev = new XMsgEvent;
    ev->setBev(bev);

    ///设置回调函数
    bufferevent_setcb(bev, read_cb, nullptr, event_cb, ev);


    int re = bufferevent_socket_connect(bev, (sockaddr *)&sin, sizeof(sin));
    if (re != 0)
    {
        std::cerr << "bufferevent_socket_connect error" << std::endl;
        return;
    }

    /// 事件主循环 监控事件是否发送， 分发事件到回调函数
    /// 如果没有事件注册则退出
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
