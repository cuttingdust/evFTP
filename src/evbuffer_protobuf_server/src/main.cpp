#include "XMsgServer.h"
#include "XMsgClient.h"
#include <iostream>

#ifndef _WIN32
#include <signal.h>
#else
#include <winsock2.h>
#endif

#define SPORT 8080

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
    XMsgClient client;
    client.setServerIp("127.0.0.1");
    client.setServerPort(SPORT);
    client.start();

    XMsgServer server;
    server.initServer(SPORT);


    return 0;
}
