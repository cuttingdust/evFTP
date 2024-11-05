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

#define SPORT        8080
#define DEFAULTINDEX "index.html"
#define WEBROOT      "."

void http_cb(struct evhttp_request *request, void *arg)
{
    std::cout << __func__ << std::endl;

    /// 1 获取浏览器的请求信息
    ///  uri
    const char *uri = evhttp_request_get_uri(request);
    std::cout << "uri:" << uri << std::endl;

    /// 请求类型 GET POST
    std::string cmdtype;
    switch (evhttp_request_get_command(request))
    {
        case evhttp_cmd_type::EVHTTP_REQ_GET:
            {
                cmdtype = "GET";
                break;
            }
        case evhttp_cmd_type::EVHTTP_REQ_POST:
            {
                cmdtype = "POST";
                break;
            }
    }
    std::cout << "cmdtype: " << cmdtype << std::endl;

    /// 消息报头
    evkeyvalq *headers = evhttp_request_get_input_headers(request);
    std::cout << "====== headers ======" << std::endl;
    for (evkeyval *p = headers->tqh_first; p != NULL; p = p->next.tqe_next)
    {
        std::cout << p->key << ":" << p->value << std::endl;
    }

    /// 请求正文 (GET为空，POST有表单信息  )
    evbuffer *inbuf     = evhttp_request_get_input_buffer(request);
    char      buf[1024] = { 0 };
    std::cout << "======= Input data ======" << std::endl;
    while (evbuffer_get_length(inbuf))
    {
        int n = evbuffer_remove(inbuf, buf, sizeof(buf) - 1);
        if (n > 0)
        {
            buf[n] = '\0';
            std::cout << buf << std::endl;
        }
    }


    /// 2 回复浏览器
    ///  状态行 消息报头 响应正文 HTTP_NOTFOUND HTTP_INTERNAL

    /// 分析出请求的文件 uri
    ///  设置根目录 WEBROOT
    std::string filepath = WEBROOT;
    filepath += uri;
    if (strcmp(uri, "/") == 0)
    {
        /// 默认加入首页文件
        filepath += DEFAULTINDEX;
    }

    /// 消息报头
    evkeyvalq *outhead = evhttp_request_get_output_headers(request);

    /// 要支持 图片 js css 下载zip文件
    /// 获取文件的后缀
    /// ./root/index.html
    int  pos    = filepath.rfind('.');
    auto suffix = filepath.substr(pos + 1, filepath.size() - (pos + 1));
    // std::cout << "suffix = " << suffix << std::endl;
    if (suffix == "jpg" || suffix == "gif" || suffix == "png" || suffix == "bmp")
    {
        std::string tmp = "image/" + suffix;
        evhttp_add_header(outhead, "Content-Type", tmp.c_str());
    }
    else if (suffix == "zip")
    {
        evhttp_add_header(outhead, "Content-Type", "application/zip");
    }
    else if (suffix == "js")
    {
        evhttp_add_header(outhead, "Content-Type", "application/x-javascript");
    }
    else if (suffix == "css")
    {
        evhttp_add_header(outhead, "Content-Type", "text/css");
    }
    else if (suffix == "html")
    {
        evhttp_add_header(outhead, "Content-Type", "text/html;charset=UTF8");
    }


    /// 读取html文件返回正文
    FILE *fp = fopen(filepath.c_str(), "rb");
    if (!fp)
    {
        std::cout << "open file failed!" << std::endl;
        evhttp_send_reply(request, HTTP_NOTFOUND, "", 0);
        return;
    }

    evbuffer *outbuf = evhttp_request_get_output_buffer(request);
    for (;;)
    {
        int len = fread(buf, 1, sizeof(buf), fp);
        if (len <= 0)
            break;
        evbuffer_add(outbuf, buf, len);
    }
    fclose(fp);
    evhttp_send_reply(request, HTTP_OK, "", outbuf);
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

    /// http 服务器
    /// 1 创建evhttp 上下文
    evhttp *evh = evhttp_new(base);

    /// 2 绑定端口和IP
    if (evhttp_bind_socket(evh, "0.0.0.0", SPORT) != 0)
    {
        std::cout << "evhttp_bind_socket failed!" << std::endl;
    }

    /// 3 设定回调函数
    evhttp_set_gencb(evh, http_cb, 0);

    /// 事件分发处理
    if (base)
        event_base_dispatch(base);
    if (evh)
    {
        evhttp_free(evh);
    }
    if (base)
    {
        event_base_free(base);
    }
    std::cin.get();
    return 0;
}
