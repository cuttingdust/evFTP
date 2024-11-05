#include <event2/event.h>
#include <event2/listener.h>
#include <event2/http.h>
#include <event2/keyvalq_struct.h>
#include <event2/bufferevent.h>
#include <event2/buffer.h>

#include <iostream>
#include <filesystem>


#ifndef _WIN32
#include <signal.h>
#endif

#define SPORT 8080

void http_client_cb(struct evhttp_request *req, void *arg)
{
    std::cout << __func__ << std::endl;
    bufferevent *bev = (bufferevent *)arg;

    /// 服务端响应错误
    if (req == nullptr)
    {
        int errcode = EVUTIL_SOCKET_ERROR();
        std::cerr << "socket error: " << evutil_socket_error_to_string(errcode) << std::endl;
        return;
    }

    /// 获取path
    const char *path = evhttp_request_get_uri(req);
    std::cout << "request path is " << path << std::endl;

    /// 获取返回的code 200 404
    std::cout << "Response: " << evhttp_request_get_response_code(req) << " " ///200
              << evhttp_request_get_response_code_line(req) << std::endl;     /// OK

    evbuffer *input = evhttp_request_get_input_buffer(req);
    for (;;)
    {
        char buf[1024] = { 0 };
        int  len       = evbuffer_remove(input, buf, sizeof(buf) - 1);
        if (len <= 0)
            break;
        buf[len] = 0;
        std::cout << buf << std::flush;
    }
}


int main(int argc, char *argv[])
{
#ifdef _WIN32
    WSADATA wsaData;
    WSAStartup(MAKEWORD(2, 2), &wsaData);

#else
    /// 忽略管道信号，发送数据给已关闭的socket
    if (signal(SIGPIPE, SIG_IGN) == SIG_ERR)
        return 1;
#endif

    std::cout << "server start!\n";
    ///创建libevent的上下文
    event_base *base = event_base_new();
    if (base)
    {
        std::cout << "event_base_new success!" << std::endl;
    }

    /// 生成请求信息 GET
    // std::string http_url = "http://ffmpeg.club/index.html?id=1";
    std::string http_url = "http://ffmpeg.club/index39139011.html&id=990909";

    /// 分析url地址
    ///  uri
    evhttp_uri *uri = evhttp_uri_parse(http_url.c_str());

    /// http https
    const char *scheme = evhttp_uri_get_scheme(uri);
    if (!scheme)
    {
        std::cerr << "scheme is null" << std::endl;
        return -1;
    }
    std::cout << "scheme: " << scheme << std::endl;

    /// port 80
    int port = evhttp_uri_get_port(uri);
    if (port < 0)
    {
        if (strcmp(scheme, "http") == 0)
            port = 80;
    }
    std::cout << "port: " << port << std::endl;

    /// host ffmpeg.club
    const char *host = evhttp_uri_get_host(uri);
    if (!host)
    {
        std::cerr << "host is null" << std::endl;
        return -1;
    }
    std::cout << "host: " << host << std::endl;

    /// path /index.html
    const char *path = evhttp_uri_get_path(uri);
    if (!path || strlen(path) == 0)
    {
        path = "/";
    }
    std::cout << "path: " << path << std::endl;

    /// ?id=1  后面的内容 id=1
    const char *query = evhttp_uri_get_query(uri);
    if (!query)
    {
        std::cout << "query is NULL" << std::endl;
    }
    else
    {
        std::cout << "query: " << query << std::endl;
    }


    /// bufferevent  连接http服务器
    bufferevent       *bev   = bufferevent_socket_new(base, -1, BEV_OPT_CLOSE_ON_FREE);
    evhttp_connection *evcon = evhttp_connection_base_bufferevent_new(base, NULL, bev, host, port);

    /// http client 请求 回调函数设置
    evhttp_request *req = evhttp_request_new(http_client_cb, bev);

    /// 设置请求的head 消息报头 信息
    evkeyvalq *output_headers = evhttp_request_get_output_headers(req);
    evhttp_add_header(output_headers, "Host", host);

    /// 发起请求
    evhttp_make_request(evcon, req, EVHTTP_REQ_GET, path);


    /// 事件分发处理
    if (base)
        event_base_dispatch(base);
    if (base)
    {
        event_base_free(base);
    }

    return 0;
}
