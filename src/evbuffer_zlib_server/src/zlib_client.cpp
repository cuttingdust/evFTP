#include <event2/event.h>
#include <event2/listener.h>
#include <event2/bufferevent.h>
#include <event2/buffer.h>
#include <iostream>

#define FILEPATH "001.txt"

struct ClientStatus
{
    FILE* fp  = 0;
    bool  end = false;
};

bufferevent_filter_result filter_out(evbuffer* s, evbuffer* d, ev_ssize_t limit, bufferevent_flush_mode mode, void* arg)
{
    std::cout << "[Client]: "
              << "client filter_out" << std::endl;
    char data[1024] = { 0 };
    int  len        = evbuffer_remove(s, data, sizeof(data));
    evbuffer_add(d, data, len);

    return BEV_OK;
}

void client_read_cb(bufferevent* bev, void* arg)
{
    /// 002 ���շ���˷��͵�OK�ظ�
    char data[1024] = { 0 };
    int  len        = bufferevent_read(bev, data, sizeof(data) - 1);
    if (strcmp(data, "OK") == 0)
    {
        std::cout << data << std::endl;
        /// ��ʼ�����ļ�,����д��ص�
        bufferevent_trigger(bev, EV_WRITE, 0);
    }
    else
    {
        bufferevent_free(bev);
    }
    std::cout << "client_read_cb " << len << std::endl;
}

void client_write_cb(bufferevent* bev, void* arg)
{
    ClientStatus* s  = (ClientStatus*)arg;
    FILE*         fp = s->fp;

    /// �ж�ʲôʱ��������Դ
    if (s->end)
    {
        /// �жϻ����Ƿ������ݣ������ˢ��
        /// ��ȡ�������󶨵�buffer
        bufferevent* be = bufferevent_get_underlying(bev);
        /// ��ȡ������弰���С
        evbuffer* evb = bufferevent_get_output(be);
        int       len = evbuffer_get_length(evb);
        std::cout << "[Client]: " << __func__ << " "
                  << "evbuffer_get_length = " << len << std::endl;

        if (len <= 0)
        {
            /// �������� �������������, ���ᷢ��
            bufferevent_free(bev);
            delete s;
            return;
        }

        /// ˢ�»���
        bufferevent_flush(bev, EV_WRITE, BEV_FINISHED);
        return;
    }

    if (!fp)
    {
        return;
    }

    std::cout << "[Client]: "
              << "client_write_cb" << std::endl;
    /// ��ȡ�ļ�����
    char data[1024] = { 0 };
    int  len        = fread(data, 1, sizeof(data), fp);
    if (len <= 0)
    {
        fclose(fp);
        s->end = true;

        /// ˢ�»�����
        bufferevent_flush(bev, EV_WRITE, BEV_FINISHED);
        return;
    }

    /// �����ļ�����
    bufferevent_write(bev, data, len);
}

void client_event_cb(bufferevent* bev, short events, void* arg)
{
    if (events & BEV_EVENT_CONNECTED)
    {
        std::cout << "[Client]: "
                  << "Client has connected" << std::endl;

        /// 001 �����ļ���
        bufferevent_write(bev, FILEPATH, strlen(FILEPATH));

        /// �����������
        bufferevent* bev_filter =
                bufferevent_filter_new(bev, 0, filter_out, BEV_OPT_CLOSE_ON_FREE | BEV_OPT_DEFER_CALLBACKS, 0, 0);
        FILE* fp = fopen(FILEPATH, "rb");
        if (!fp)
        {
            std::cout << "[Client]: "
                      << "fopen" << FILEPATH << " failed" << std::endl;
            // return;
        }
        ClientStatus* s = new ClientStatus();
        s->fp           = fp;

        /// ���ûص�
        bufferevent_setcb(bev_filter, client_read_cb, client_write_cb, client_event_cb, s);
        bufferevent_enable(bev_filter, EV_READ | EV_WRITE);
    }
    else if (events & BEV_EVENT_EOF)
    {
        std::cout << "[Client]: "
                  << "Connection closed" << std::endl;
    }
    else if (events & BEV_EVENT_ERROR)
    {
        std::cout << "[Client]: "
                  << "Some other error" << std::endl;
    }
}


void Client(event_base* base)
{
    std::cout << "Zlib Client Start" << std::endl;

    ///1. ���ӷ����
    sockaddr_in sin;
    memset(&sin, 0, sizeof(sin));
    sin.sin_family = AF_INET;
    sin.sin_port   = htons(8080);
    evutil_inet_pton(AF_INET, "127.0.0.1", &sin.sin_addr);
    bufferevent* bev = bufferevent_socket_new(base, -1, BEV_OPT_CLOSE_ON_FREE);

    ///2.ֻ���¼��ص�������ȷ�����ӳɹ�
    bufferevent_enable(bev, EV_READ | EV_WRITE);
    bufferevent_setcb(bev, 0, 0, client_event_cb, 0);

    ///3.�յ��ظ�ȷ��OK
    bufferevent_socket_connect(bev, (sockaddr*)&sin, sizeof(sin));
}
