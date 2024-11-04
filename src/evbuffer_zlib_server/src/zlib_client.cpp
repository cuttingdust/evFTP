#include <event2/event.h>
#include <event2/listener.h>
#include <event2/bufferevent.h>
#include <event2/buffer.h>
#include <iostream>
#include <zlib.h>

#define FILEPATH "001.txt"

struct ClientStatus
{
    FILE*     fp        = 0;
    bool      end       = false;
    bool      startSend = false;
    z_stream* z_output  = 0;
    int       readNum   = 0;
    int       sendNum   = 0;
    ~ClientStatus()
    {
        if (fp)
        {
            fclose(fp);
            fp = 0;
        }
        if (z_output)
        {
            deflateEnd(z_output);
            delete z_output;
            z_output = 0;
        }
        readNum = 0;
        sendNum = 0;
    }
};

bufferevent_filter_result filter_out(evbuffer* s, evbuffer* d, ev_ssize_t limit, bufferevent_flush_mode mode, void* arg)
{
    ClientStatus* sta = (ClientStatus*)arg;
    std::cout << "[Client]: "
              << "client filter_out" << std::endl;

    /// ѹ���ļ�
    ///  �����ļ���Ϣ001 ȥ��
    if (!sta->startSend)
    {
        char data[1024] = { 0 };
        int  len        = evbuffer_remove(s, data, sizeof(data));
        evbuffer_add(d, data, len);
        return BEV_OK;
    }

    /// ��ʼѹ���ļ�
    ///  ȡ��buffer�����ݵ�����
    evbuffer_iovec v_in[1];
    int            n = evbuffer_peek(s, -1, 0, v_in, 1);
    if (n <= 0)
    {
        /// ����write �ص� ����ռ�
        if (sta->end)
        {
            return BEV_OK;
        }

        /// û������ BEV_NEED_MORE �������д��ص�
        return BEV_NEED_MORE;
    }
    z_stream* p = sta->z_output;
    if (!p)
    {
        return BEV_ERROR;
    }
    /// zlib �������ݴ�С
    p->avail_in = v_in[0].iov_len;
    /// zlib  �������ݵ�ַ
    p->next_in = (Byte*)v_in[0].iov_base;

    /// ��������ռ��С
    evbuffer_iovec v_out[1];
    evbuffer_reserve_space(d, 4096, v_out, 1);

    /// zlib ����ռ��С
    p->avail_out = v_out[0].iov_len;
    /// zlib ����ռ��ַ
    p->next_out = (Byte*)v_out[0].iov_base;

    /// zlib ѹ��
    int ret = deflate(p, Z_SYNC_FLUSH);
    if (ret != Z_OK)
    {
        std::cerr << "deflate failed!" << std::endl;
    }


    /// ѹ�����˶������� ��source evbuffer���Ƴ�
    /// p->avail_in δ�������ݴ�С
    size_t nread = v_in[0].iov_len - p->avail_in;

    /// ѹ�������ݴ�С ���� des evbuffer
    ///  p->avail_out ʣ��ռ��С
    size_t nwrite = v_out[0].iov_len - p->avail_out;

    /// �Ƴ�source evbuffer������
    evbuffer_drain(s, nread);

    /// ����des evbuffer
    v_out[0].iov_len = nwrite;
    evbuffer_commit_space(d, v_out, 1);
    std::cout << "[Client]: "
              << "nread: " << nread << " nwrite: " << nwrite << std::endl;
    sta->readNum += nread;
    sta->sendNum += nwrite;

    return BEV_OK;
}

void client_read_cb(bufferevent* bev, void* arg)
{
    ClientStatus* sta = (ClientStatus*)arg;

    /// 002 ���շ���˷��͵�OK�ظ�
    char   data[1024] = { 0 };
    size_t len        = bufferevent_read(bev, data, sizeof(data) - 1);
    if (strcmp(data, "OK") == 0)
    {
        std::cout << "[Client]: " << data << std::endl;
        sta->startSend = true;
        /// ��ʼ�����ļ�,����д��ص�
        bufferevent_trigger(bev, EV_WRITE, 0);
    }
    else
    {
        bufferevent_free(bev);
    }
    std::cout << "[Client]: "
              << "client_read_cb " << len << std::endl;
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
        size_t    len = evbuffer_get_length(evb);
        std::cout << "[Client]: " << __func__ << " "
                  << "evbuffer_get_length = " << len << std::endl;

        if (len <= 0)
        {
            std::cout << "[Client]: "
                      << "read " << s->readNum << " "
                      << "send " << s->sendNum << std::endl;

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
    char   data[1024] = { 0 };
    size_t len        = fread(data, 1, sizeof(data), fp);
    if (len <= 0)
    {
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


        FILE* fp = fopen(FILEPATH, "rb");
        if (!fp)
        {
            std::cout << "[Client]: "
                      << "fopen" << FILEPATH << " failed" << std::endl;
            // return;
        }
        ClientStatus* s = new ClientStatus();
        s->fp           = fp;
        s->z_output     = new z_stream();
        deflateInit(s->z_output, Z_DEFAULT_COMPRESSION);


        /// �����������
        bufferevent* bev_filter =
                bufferevent_filter_new(bev, 0, filter_out, BEV_OPT_CLOSE_ON_FREE | BEV_OPT_DEFER_CALLBACKS, 0, s);

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
