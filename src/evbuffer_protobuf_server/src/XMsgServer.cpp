#include "XMsgServer.h"

#include "XMsgCom.pb.h"
#include "XMsgEvent.h"

#include <iostream>

#include <event2/event.h>
#include <event2/listener.h>
#include <event2/bufferevent.h>

static void read_cb(struct bufferevent *be, void *arg)
{
    std::cout << "[SR]:" << std::flush;
    auto ev = static_cast<XMsgEvent *>(arg);

    if (!ev->recvMsg())
    {
        ev->clear();
        delete ev;
        bufferevent_free(be);
        return;
    }
    auto msg = ev->getMsg();
    if (!msg)
    {
        return;
    }

    /// �����л�
    XMsg::XLoginReq req;
    req.ParseFromArray(msg->data, msg->size);
    std::cout << "Recv username = " << req.username() << std::endl;
    std::cout << "Recv password = " << req.password() << std::endl;

    /// ������Ϣ
    XMsg::XLoginRes res;
    res.set_restype(XMsg::XLoginRes::XRT_OK);
    std::string token = req.username();
    token += " sign.";
    res.set_token(token);
    ev->sendMsg(XMsg::MT_LOGIN_RES, &res);
    ev->clear();

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
    //
    // /// �������� д�뵽�������
    // bufferevent_write(be, "OK", 3);
}

/// ���󣬳�ʱ �����ӶϿ�����룩
static void event_cb(struct bufferevent *be, short events, void *arg)
{
    std::cout << "[SE]:" << std::flush;
    const auto ev = static_cast<XMsgEvent *>(arg);

    if (events & (BEV_EVENT_TIMEOUT | BEV_EVENT_ERROR | BEV_EVENT_EOF))
    {
        std::cout << " BEV_EVENT_TIMEOUT | BEV_EVENT_ERROR  | BEV_EVENT_EOF" << std::endl;
        delete ev;
        bufferevent_free(be);
    }
}

static void listen_cb(struct evconnlistener *evc, evutil_socket_t client_socket, struct sockaddr *client_addr,
                      int socklen, void *arg)
{
    std::cout << "listen_cb" << std::endl;

    char         ip[16] = { 0 };
    sockaddr_in *addr   = (sockaddr_in *)client_addr;
    evutil_inet_ntop(AF_INET, &addr->sin_addr, ip, sizeof(ip));
    std::cout << "client ip is " << ip << std::endl;

    event_base *base = (event_base *)arg;

    /// ����bufferevent������ BEV_OPT_CLOSE_ON_FREE����buffereventʱ�ر�socket
    bufferevent *bev = bufferevent_socket_new(base, client_socket, BEV_OPT_CLOSE_ON_FREE);
    if (!bev)
    {
        std::cerr << "bufferevent_socket_new failed!" << std::endl;
        return;
    }

    /// ���Ӽ���¼�
    bufferevent_enable(bev, EV_READ | EV_WRITE);

    /// ��ʱʱ�������
    timeval t1 = { 30, 0 };
    bufferevent_set_timeouts(bev, &t1, nullptr);

    auto ev = new XMsgEvent();
    ev->setBev(bev);

    /// ���ûص�����
    bufferevent_setcb(bev, read_cb, 0, event_cb, ev);
}

XMsgServer::XMsgServer() = default;

XMsgServer::~XMsgServer() = default;

void XMsgServer::initServer(int serverPort)
{
    std::cout << "XMsgServer::initServer" << std::endl;
    ///����libevent��������
    event_base *base = event_base_new();
    if (base)
    {
        std::cout << "event_base_new success!" << std::endl;
    }

    /// �����˿�
    /// socket, bind, listen ���¼�
    sockaddr_in sin;
    memset(&sin, 0, sizeof(sin));
    sin.sin_family = AF_INET;
    sin.sin_port   = htons(serverPort);

    evconnlistener *ev = evconnlistener_new_bind(
            base,                                      ///  libevent��������
            listen_cb,                                 /// ���յ����ӵĻص�����
            base,                                      /// �ص�������ȡ�Ĳ��� arg
            LEV_OPT_REUSEABLE | LEV_OPT_CLOSE_ON_FREE, /// ��ַ���ã�evconnlistener�ر�ͬʱ�ر�socket
            10,                                        /// ���Ӷ��д�С����Ӧlisten����
            (sockaddr *)&sin,                          /// �󶨵ĵ�ַ�Ͷ˿�
            sizeof(sin));

    /// �¼��ַ�����
    if (base)
        event_base_dispatch(base);
    if (ev)
        evconnlistener_free(ev);
    if (base)
        event_base_free(base);
}