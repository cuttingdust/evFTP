#include <event2/event.h>
#include <event2/listener.h>
#include <event2/http.h>
#include <event2/keyvalq_struct.h>
#include <event2/buffer.h>
#include <iostream>

#ifndef _WIN32
#include <signal.h>
#endif

#define SPORT 8080

void http_cb(struct evhttp_request *request, void *arg)
{
    std::cout << __func__ << std::endl;

    /// 1 ��ȡ�������������Ϣ
    ///  uri
    const char *uri = evhttp_request_get_uri(request);
    std::cout << "uri:" << uri << std::endl;

    /// �������� GET POST
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

    /// ��Ϣ��ͷ
    evkeyvalq *headers = evhttp_request_get_input_headers(request);
    std::cout << "====== headers ======" << std::endl;
    for (evkeyval *p = headers->tqh_first; p != NULL; p = p->next.tqe_next)
    {
        std::cout << p->key << ":" << p->value << std::endl;
    }

    /// �������� (GETΪ�գ�POST�б���Ϣ  )
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


    /// 2 �ظ������
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

    std::cout << "server start!\n";
    ///����libevent��������
    event_base *base = event_base_new();
    if (base)
    {
        std::cout << "event_base_new success!" << std::endl;
    }

    /// http ������
    /// 1 ����evhttp ������
    evhttp *evh = evhttp_new(base);

    /// 2 �󶨶˿ں�IP
    if (evhttp_bind_socket(evh, "0.0.0.0", SPORT) != 0)
    {
        std::cout << "evhttp_bind_socket failed!" << std::endl;
    }

    /// 3 �趨�ص�����
    evhttp_set_gencb(evh, http_cb, 0);

    /// �¼��ַ�����
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

    return 0;
}
