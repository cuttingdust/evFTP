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

    /// 压缩文件
    ///  发送文件消息001 去掉
    if (!sta->startSend)
    {
        char data[1024] = { 0 };
        int  len        = evbuffer_remove(s, data, sizeof(data));
        evbuffer_add(d, data, len);
        return BEV_OK;
    }

    /// 开始压缩文件
    ///  取出buffer中数据的引用
    evbuffer_iovec v_in[1];
    int            n = evbuffer_peek(s, -1, 0, v_in, 1);
    if (n <= 0)
    {
        /// 调用write 回调 清理空间
        if (sta->end)
        {
            return BEV_OK;
        }

        /// 没有数据 BEV_NEED_MORE 不会进入写入回调
        return BEV_NEED_MORE;
    }
    z_stream* p = sta->z_output;
    if (!p)
    {
        return BEV_ERROR;
    }
    /// zlib 输入数据大小
    p->avail_in = v_in[0].iov_len;
    /// zlib  输入数据地址
    p->next_in = (Byte*)v_in[0].iov_base;

    /// 申请输出空间大小
    evbuffer_iovec v_out[1];
    evbuffer_reserve_space(d, 4096, v_out, 1);

    /// zlib 输出空间大小
    p->avail_out = v_out[0].iov_len;
    /// zlib 输出空间地址
    p->next_out = (Byte*)v_out[0].iov_base;

    /// zlib 压缩
    int ret = deflate(p, Z_SYNC_FLUSH);
    if (ret != Z_OK)
    {
        std::cerr << "deflate failed!" << std::endl;
    }


    /// 压缩用了多少数据 从source evbuffer中移除
    /// p->avail_in 未处理数据大小
    size_t nread = v_in[0].iov_len - p->avail_in;

    /// 压缩后数据大小 传入 des evbuffer
    ///  p->avail_out 剩余空间大小
    size_t nwrite = v_out[0].iov_len - p->avail_out;

    /// 移除source evbuffer中数据
    evbuffer_drain(s, nread);

    /// 传入des evbuffer
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

    /// 002 接收服务端发送的OK回复
    char   data[1024] = { 0 };
    size_t len        = bufferevent_read(bev, data, sizeof(data) - 1);
    if (strcmp(data, "OK") == 0)
    {
        std::cout << "[Client]: " << data << std::endl;
        sta->startSend = true;
        /// 开始发送文件,触发写入回调
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

    /// 判断什么时候清理资源
    if (s->end)
    {
        /// 判断缓冲是否有数据，如果有刷新
        /// 获取过滤器绑定的buffer
        bufferevent* be = bufferevent_get_underlying(bev);
        /// 获取输出缓冲及其大小
        evbuffer* evb = bufferevent_get_output(be);
        size_t    len = evbuffer_get_length(evb);
        std::cout << "[Client]: " << __func__ << " "
                  << "evbuffer_get_length = " << len << std::endl;

        if (len <= 0)
        {
            std::cout << "[Client]: "
                      << "read " << s->readNum << " "
                      << "send " << s->sendNum << std::endl;

            /// 立即清理 如果缓冲有数据, 不会发送
            bufferevent_free(bev);
            delete s;


            return;
        }

        /// 刷新缓冲
        bufferevent_flush(bev, EV_WRITE, BEV_FINISHED);
        return;
    }

    if (!fp)
    {
        return;
    }

    std::cout << "[Client]: "
              << "client_write_cb" << std::endl;
    /// 读取文件内容
    char   data[1024] = { 0 };
    size_t len        = fread(data, 1, sizeof(data), fp);
    if (len <= 0)
    {
        s->end = true;

        /// 刷新缓冲区
        bufferevent_flush(bev, EV_WRITE, BEV_FINISHED);
        return;
    }

    /// 发送文件内容
    bufferevent_write(bev, data, len);
}

void client_event_cb(bufferevent* bev, short events, void* arg)
{
    if (events & BEV_EVENT_CONNECTED)
    {
        std::cout << "[Client]: "
                  << "Client has connected" << std::endl;

        /// 001 发送文件名
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


        /// 创建输出过滤
        bufferevent* bev_filter =
                bufferevent_filter_new(bev, 0, filter_out, BEV_OPT_CLOSE_ON_FREE | BEV_OPT_DEFER_CALLBACKS, 0, s);

        /// 设置回调
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

    ///1. 连接服务端
    sockaddr_in sin;
    memset(&sin, 0, sizeof(sin));
    sin.sin_family = AF_INET;
    sin.sin_port   = htons(8080);
    evutil_inet_pton(AF_INET, "127.0.0.1", &sin.sin_addr);
    bufferevent* bev = bufferevent_socket_new(base, -1, BEV_OPT_CLOSE_ON_FREE);

    ///2.只绑定事件回调，用来确认连接成功
    bufferevent_enable(bev, EV_READ | EV_WRITE);
    bufferevent_setcb(bev, 0, 0, client_event_cb, 0);

    ///3.收到回复确认OK
    bufferevent_socket_connect(bev, (sockaddr*)&sin, sizeof(sin));
}
