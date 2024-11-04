#include <event2/event.h>
#include <event2/listener.h>
#include <event2/bufferevent.h>
#include <event2/buffer.h>
#include <zlib.h>

#include <iostream>
#include <filesystem>

#include <event2/buffer.h>

#define SPORT 8080

struct Status
{
    bool      start    = false;
    FILE     *fp       = 0;
    z_stream *p        = 0;
    int       recvNum  = 0;
    int       writeNum = 0;
    ~Status()
    {
        if (fp)
        {
            fclose(fp);
            fp = 0;
        }

        if (p)
        {
            inflateEnd(p);
            delete p;
            p = 0;
        }
        recvNum  = 0;
        writeNum = 0;
    }
};


bufferevent_filter_result filter_in(struct evbuffer *src, struct evbuffer *dst, ev_ssize_t dst_limit,
                                    enum bufferevent_flush_mode mode, void *arg)
{
    std::cout << "[Server]: " << __func__ << std::endl;
    Status *status = (Status *)arg;
    /// 1. ���ܿͻ��˷��͵��ļ���
    if (!status->start)
    {
        char data[1024] = { 0 };
        int  len        = evbuffer_remove(src, data, sizeof(data) - 1);
        std::cout << "[Server]: "
                  << "server recv " << data << std::endl;
        evbuffer_add(dst, data, len);
        return BEV_OK;
    }

    /// ��ѹ
    evbuffer_iovec v_in[1];
    /// ��ȡ���� ��������
    int n = evbuffer_peek(src, -1, nullptr, v_in, 1);
    if (n <= 0)
    {
        return BEV_NEED_MORE;
    }

    /// ��ѹ������
    z_stream *p = status->p;
    if (!p)
    {
        return BEV_ERROR;
    }
    /// zlib �������ݴ�С
    p->avail_in = v_in[0].iov_len;
    /// zlib  �������ݵ�ַ
    p->next_in = (Byte *)v_in[0].iov_base;

    /// ��������ռ��С
    evbuffer_iovec v_out[1];
    evbuffer_reserve_space(dst, 4096, v_out, 1);

    /// zlib ����ռ��С
    p->avail_out = v_out[0].iov_len;
    /// zlib ����ռ��ַ
    p->next_out = (Byte *)v_out[0].iov_base;

    int re = inflate(p, Z_SYNC_FLUSH);
    if (re != Z_OK)
    {
        std::cerr << "deflare failed!" << std::endl;
    }

    /// ��ѹ���˶������� ��source evbuffer���Ƴ�
    /// p->avail_in δ�������ݴ�С
    size_t nread = v_in[0].iov_len - p->avail_in;

    /// ��ѹ�����ݴ�С ���� des evbuffer
    ///  p->avail_out ʣ��ռ��С
    size_t nwrite = v_out[0].iov_len - p->avail_out;

    /// �Ƴ�source evbuffer������
    evbuffer_drain(src, nread);

    /// ����des evbuffer
    v_out[0].iov_len = nwrite;
    evbuffer_commit_space(dst, v_out, 1);
    std::cout << "[Server]: "
              << "nread: " << nread << " nwrite: " << nwrite << std::endl;

    status->recvNum += nread;
    status->writeNum += nwrite;

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
        char   data[1024] = { 0 };
        size_t len        = bufferevent_read(bev, data, sizeof(data));
        if (len >= 0)
        {
            fwrite(data, 1, len, status->fp);
            fflush(status->fp);
        }
    }
    while (evbuffer_get_length(bufferevent_get_input(bev)) > 0);
}

void event_cb(bufferevent *bev, short events, void *arg)
{
    std::cout << "[Server]: "
              << "event_cb" << std::endl;
    Status *status = (Status *)arg;
    if (events & BEV_EVENT_EOF)
    {
        std::cout << "[Server]: "
                  << "connection closed" << std::endl;
        std::cout << "[Server]: "
                  << "recv " << status->recvNum << " "
                  << "write " << status->writeNum << std::endl;
        /// ������Դ
        if (status)
        {
            delete status;
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
    status->p           = new z_stream();
    inflateInit(status->p);

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
