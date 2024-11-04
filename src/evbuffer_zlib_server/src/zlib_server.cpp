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
    /// 1. 接受客户端发送的文件名
    if (!status->start)
    {
        char data[1024] = { 0 };
        int  len        = evbuffer_remove(src, data, sizeof(data) - 1);
        std::cout << "[Server]: "
                  << "server recv " << data << std::endl;
        evbuffer_add(dst, data, len);
        return BEV_OK;
    }

    /// 解压
    evbuffer_iovec v_in[1];
    /// 读取数据 不清理缓冲
    int n = evbuffer_peek(src, -1, nullptr, v_in, 1);
    if (n <= 0)
    {
        return BEV_NEED_MORE;
    }

    /// 解压上下文
    z_stream *p = status->p;
    if (!p)
    {
        return BEV_ERROR;
    }
    /// zlib 输入数据大小
    p->avail_in = v_in[0].iov_len;
    /// zlib  输入数据地址
    p->next_in = (Byte *)v_in[0].iov_base;

    /// 申请输出空间大小
    evbuffer_iovec v_out[1];
    evbuffer_reserve_space(dst, 4096, v_out, 1);

    /// zlib 输出空间大小
    p->avail_out = v_out[0].iov_len;
    /// zlib 输出空间地址
    p->next_out = (Byte *)v_out[0].iov_base;

    int re = inflate(p, Z_SYNC_FLUSH);
    if (re != Z_OK)
    {
        std::cerr << "deflare failed!" << std::endl;
    }

    /// 解压用了多少数据 从source evbuffer中移除
    /// p->avail_in 未处理数据大小
    size_t nread = v_in[0].iov_len - p->avail_in;

    /// 解压后数据大小 传入 des evbuffer
    ///  p->avail_out 剩余空间大小
    size_t nwrite = v_out[0].iov_len - p->avail_out;

    /// 移除source evbuffer中数据
    evbuffer_drain(src, nread);

    /// 传入des evbuffer
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
        /// 001 接受文件名
        char data[1024] = { 0 };
        bufferevent_read(bev, data, sizeof(data) - 1);
        std::string out = "out\\";
        out += data;

        /// 先判断文件存不存在 不存在 就创建
        std::filesystem::path path(out);
        if (!std::filesystem::exists(path.parent_path()))
        {
            std::filesystem::create_directories(path.parent_path());
        }

        /// 打开写入文件
        status->fp = fopen(out.c_str(), "wb");
        if (!status->fp)
        {
            std::cout << "[Server]: "
                      << "server open " << out << " failed!" << std::endl;
            return;
        }

        /// 002 回复OK
        bufferevent_write(bev, "OK", 2);
        status->start = true;
        return;
    }

    do
    {
        /// 写入文件
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
        /// 清理资源
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

    /// 1. 创建一个bufferevent 用来通信
    bufferevent *bev    = bufferevent_socket_new(base, s, BEV_OPT_CLOSE_ON_FREE);
    Status      *status = new Status();
    status->p           = new z_stream();
    inflateInit(status->p);

    /// 2. 添加过滤 并设置输入回调
    bufferevent *bev_filter = bufferevent_filter_new(bev,                   /// bufferevent
                                                     filter_in,             /// 输入过滤函数
                                                     0,                     /// 输出过滤
                                                     BEV_OPT_CLOSE_ON_FREE, /// 关闭filter同时管理bufferevent
                                                     0,                     /// 清理回调
                                                     status                 /// 传递参数
    );

    /// 3. 设置回调 读取 事件（处理连接断开）
    bufferevent_setcb(bev_filter, read_cb, 0, event_cb, status);
    bufferevent_enable(bev_filter, EV_READ | EV_WRITE);
}


void Server(event_base *base)
{
    std::cout << "Zlib Server Start" << std::endl;

    /// 监听端口
    /// socket, bind, listen 绑定事件
    sockaddr_in sin;
    memset(&sin, 0, sizeof(sin));
    sin.sin_family = AF_INET;
    sin.sin_port   = htons(SPORT);

    evconnlistener *ev = evconnlistener_new_bind(
            base,                                      ///  libevent的上下文
            listen_cb,                                 /// 接收到连接的回调函数
            base,                                      /// 回调函数获取的参数 arg
            LEV_OPT_REUSEABLE | LEV_OPT_CLOSE_ON_FREE, /// 地址重用，evconnlistener关闭同时关闭socket
            10,                                        /// 连接队列大小，对应listen函数
            (sockaddr *)&sin,                          /// 绑定的地址和端口
            sizeof(sin));
}
