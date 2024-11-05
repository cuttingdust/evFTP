#include <event2/event.h>
#include <event2/listener.h>
#include <event2/http.h>
#include <event2/keyvalq_struct.h>
#include <event2/buffer.h>
#include <iostream>
#include <filesystem>

#ifndef _WIN32
#include <signal.h>
#endif

#define SPORT 8080

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
    std::string http_url = "http://ffmpeg.club/index.html?id=1";

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
    std::cout << "query: " << query << std::endl;

    /// 事件分发处理
    if (base)
        event_base_dispatch(base);
    if (base)
    {
        event_base_free(base);
    }

    return 0;
}
