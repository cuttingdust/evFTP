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
    event_base *base = (event_base *)arg;

    /// �������Ӧ����
    if (req == nullptr)
    {
        int errcode = EVUTIL_SOCKET_ERROR();
        std::cerr << "socket error: " << evutil_socket_error_to_string(errcode) << std::endl;
        return;
    }

    /// ��ȡpath
    const char *path = evhttp_request_get_uri(req);
    std::cout << "request path is " << path << std::endl;
    std::string filepath = ".";
    filepath += path;

    /// ���·������Ŀ¼����Ҫ������Ŀ¼��������
    std::filesystem::path file_path(filepath);
    std::filesystem::path dir_path = file_path.parent_path();
    if (!std::filesystem::exists(dir_path))
    {
        std::filesystem::create_directories(dir_path);
    }

    FILE *fp = fopen(filepath.c_str(), "wb");
    if (!fp)
    {
        std::cerr << "open file " << filepath << " failed!" << std::endl;
    }

    /// ��ȡ���ص�code 200 404
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
        if (!fp)
            continue;
        fwrite(buf, 1, len, fp);
        std::cout << buf << std::flush;
    }

    if (fp)
        fclose(fp);

    event_base_loopbreak(base);
}


void testGetHttp()
{
    ///����libevent��������
    event_base *base = event_base_new();
    if (base)
    {
        std::cout << "event_base_new success!" << std::endl;
    }

    /// ����������Ϣ GET
    std::string http_url = "http://ffmpeg.club/index.html?id=1";
    http_url             = "http://ffmpeg.club/index39139011.html&id=990909"; /// NOT FOUND
    http_url             = "http://ffmpeg.club/101.jpg";                      /// ͼƬ��ʧЧ
    http_url             = "http://img.keaitupian.cn/newupload/06/1687339478627284.jpg";
    ///////////////////////////////////////////////////////////////////////////
    /// ����url��ַ
    ///  uri
    evhttp_uri *uri = evhttp_uri_parse(http_url.c_str());

    /// http https
    const char *scheme = evhttp_uri_get_scheme(uri);
    if (!scheme)
    {
        std::cerr << "scheme is null" << std::endl;
        return;
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
        return;
    }
    std::cout << "host: " << host << std::endl;

    /// path /index.html
    const char *path = evhttp_uri_get_path(uri);
    if (!path || strlen(path) == 0)
    {
        path = "/";
    }
    std::cout << "path: " << path << std::endl;

    /// ?id=1  ��������� id=1
    const char *query = evhttp_uri_get_query(uri);
    if (!query)
    {
        std::cout << "query is NULL" << std::endl;
    }
    else
    {
        std::cout << "query: " << query << std::endl;
    }


    /// bufferevent  ����http������
    bufferevent       *bev   = bufferevent_socket_new(base, -1, BEV_OPT_CLOSE_ON_FREE);
    evhttp_connection *evcon = evhttp_connection_base_bufferevent_new(base, NULL, bev, host, port);

    /// http client ���� �ص���������
    evhttp_request *req = evhttp_request_new(http_client_cb, bev);

    /// ���������head ��Ϣ��ͷ ��Ϣ
    evkeyvalq *output_headers = evhttp_request_get_output_headers(req);
    evhttp_add_header(output_headers, "Host", host);

    /// ��������
    evhttp_make_request(evcon, req, EVHTTP_REQ_GET, path);


    /// �¼��ַ�����
    if (base)
        event_base_dispatch(base);
    if (uri)
        evhttp_uri_free(uri);
    if (evcon)
        evhttp_connection_free(evcon);
    if (base)
        event_base_free(base);
}

void testPostHttp()
{
    ///����libevent��������
    event_base *base = event_base_new();
    if (base)
    {
        std::cout << "event_base_new success!" << std::endl;
    }

    /// ����������Ϣ POST
    std::string http_url = "http://127.0.0.1:8080/index.html";
    ///////////////////////////////////////////////////////////////////////////
    /// ����url��ַ
    ///  uri
    evhttp_uri *uri = evhttp_uri_parse(http_url.c_str());

    /// http https
    const char *scheme = evhttp_uri_get_scheme(uri);
    if (!scheme)
    {
        std::cerr << "scheme is null" << std::endl;
        return;
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
        return;
    }
    std::cout << "host: " << host << std::endl;

    /// path /index.html
    const char *path = evhttp_uri_get_path(uri);
    if (!path || strlen(path) == 0)
    {
        path = "/";
    }
    std::cout << "path: " << path << std::endl;

    /// ?id=1  ��������� id=1
    const char *query = evhttp_uri_get_query(uri);
    if (!query)
    {
        std::cout << "query is NULL" << std::endl;
    }
    else
    {
        std::cout << "query: " << query << std::endl;
    }

    /// bufferevent  ����http������
    bufferevent       *bev   = bufferevent_socket_new(base, -1, BEV_OPT_CLOSE_ON_FREE);
    evhttp_connection *evcon = evhttp_connection_base_bufferevent_new(base, NULL, bev, host, port);

    /// http client ���� �ص���������
    evhttp_request *req = evhttp_request_new(http_client_cb, bev);

    /// ���������head ��Ϣ��ͷ ��Ϣ
    evkeyvalq *output_headers = evhttp_request_get_output_headers(req);
    evhttp_add_header(output_headers, "Host", host);

    /// ����post����
    evbuffer *output = evhttp_request_get_output_buffer(req);
    evbuffer_add_printf(output, "xcj=%d&b=%d", 1, 2);

    /// ��������
    evhttp_make_request(evcon, req, EVHTTP_REQ_POST, path);


    /// �¼��ַ�����
    if (base)
        event_base_dispatch(base);
    if (uri)
        evhttp_uri_free(uri);
    if (evcon)
        evhttp_connection_free(evcon);
    if (base)
        event_base_free(base);
}
int main(int argc, char *argv[])
{
#ifdef _WIN32
    WSADATA wsaData;
    WSAStartup(MAKEWORD(2, 2), &wsaData);

#else
    /// ���Թܵ��źţ��������ݸ��ѹرյ�socket
    if (signal(SIGPIPE, SIG_IGN) == SIG_ERR)
        return 1;
#endif

    std::cout << "test http client start!\n";
    testGetHttp();
    testPostHttp();

    return 0;
}
