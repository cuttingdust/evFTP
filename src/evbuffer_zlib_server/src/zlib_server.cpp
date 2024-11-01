#include <event2/event.h>
#include <event2/listener.h>
#include <event2/bufferevent.h>
#include <event2/buffer.h>

#include <iostream>
#include <filesystem>

#include <event2/buffer.h>
#include <event2/bufferevent.h>

#define SPORT 8080

struct Status
{
    bool  start = false;
    FILE *fp    = 0;
};


bufferevent_filter_result filter_in(struct evbuffer *src, struct evbuffer *dst, ev_ssize_t dst_limit,
                                    enum bufferevent_flush_mode mode, void *arg)
{
    std::cout << "[Server]: " << __func__ << std::endl;
    /// 1. ���ܿͻ��˷��͵��ļ���
    char data[1024] = { 0 };
    int  len        = evbuffer_remove(src, data, sizeof(data) - 1);
    std::cout << "[Server]: "
              << "server recv " << data << std::endl;
    evbuffer_add(dst, data, len);

    return BEV_OK;
}

void read_cb(bufferevent *bev, void *arg)
{
    std::cout << "[Server]: " << __func__ << std::endl;

    Status *status = (Status *)arg;
    if (!status->start)
    {
        /// 001 �����ļ���
        char data[1024] = { 0 };
        bufferevent_read(bev, data, sizeof(data) - 1);
        std::string out = "out\\";
        out += data;

        /// ���ж��ļ��治���� ������ �ʹ���
        std::filesystem::path path(out);
        if (!std::filesystem::exists(path.parent_path()))
        {
            std::filesystem::create_directories(path.parent_path());
        }

        /// ��д���ļ�
        status->fp = fopen(out.c_str(), "wb");
        if (!status->fp)
        {
            std::cout << "[Server]: "
                      << "server open " << out << " failed!" << std::endl;
            return;
        }

        /// 002 �ظ�OK
        bufferevent_write(bev, "OK", 2);
        status->start = true;
        return;
    }

    do
    {
        /// д���ļ�
        char data[1024] = { 0 };
        int  len        = bufferevent_read(bev, data, sizeof(data));
        if (len >= 0)
        {
            fwrite(data, 1, len, status->fp);
            fflush(status->fp);
        }
    } while (evbuffer_get_length(bufferevent_get_input(bev)) > 0);
    
}

void event_cb(bufferevent *bev, short events, void *arg)
{
    std::cout << "event_cb" << std::endl;
    Status *status = (Status *)arg;
    if (events & BEV_EVENT_EOF)
    {
        std::cout << "[Server]: " <<  "connection closed" << std::endl;
        /// ������Դ
        if (status->fp)
        {
            fclose(status->fp);
            status->fp = 0;
        }
        bufferevent_free(bev);
    }
    else if (events & BEV_EVENT_ERROR)
    {
        std::cout << "[Server]: "
                            << "some other error" << std::endl;
    }
    else if (events & BEV_EVENT_CONNECTED)
    {
        std::cout << "[Server]: "
                     "client has connected"
                  << std::endl;
    }
}

void listen_cb(struct evconnlistener *e, evutil_socket_t s, struct sockaddr *a, int socklen, void *arg)
{
    std::cout << "[Server]:" << __func__ << std::endl;
    event_base *base = (event_base *)arg;

    /// 1. ����һ��bufferevent ����ͨ��
    bufferevent *bev    = bufferevent_socket_new(base, s, BEV_OPT_CLOSE_ON_FREE);
    Status      *status = new Status();

    /// 2. ��ӹ��� ����������ص�
    bufferevent *bev_filter = bufferevent_filter_new(bev,                   /// bufferevent
                                                     filter_in,             /// ������˺���
                                                     0,                     /// �������
                                                     BEV_OPT_CLOSE_ON_FREE, /// �ر�filterͬʱ����bufferevent
                                                     0,                     /// ����ص�
                                                     status                 /// ���ݲ���
    );

    /// 3. ���ûص� ��ȡ �¼����������ӶϿ���
    bufferevent_setcb(bev_filter, read_cb, 0, event_cb, status);
    bufferevent_enable(bev_filter, EV_READ | EV_WRITE);
}


void Server(event_base *base)
{
    std::cout << "Zlib Server Start" << std::endl;

    /// �����˿�
    /// socket, bind, listen ���¼�
    sockaddr_in sin;
    memset(&sin, 0, sizeof(sin));
    sin.sin_family = AF_INET;
    sin.sin_port   = htons(SPORT);

    evconnlistener *ev = evconnlistener_new_bind(
            base,                                      ///  libevent��������
            listen_cb,                                 /// ���յ����ӵĻص�����
            base,                                      /// �ص�������ȡ�Ĳ��� arg
            LEV_OPT_REUSEABLE | LEV_OPT_CLOSE_ON_FREE, /// ��ַ���ã�evconnlistener�ر�ͬʱ�ر�socket
            10,                                        /// ���Ӷ��д�С����Ӧlisten����
            (sockaddr *)&sin,                          /// �󶨵ĵ�ַ�Ͷ˿�
            sizeof(sin));
}
