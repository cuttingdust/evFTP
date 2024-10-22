#include <event2/event.h>
#include <iostream>


int main(int argc, char *argv[])
{
#ifdef _WIN32
    WSADATA wsaData;
    WSAStartup(MAKEWORD(2, 2), &wsaData);
#endif

    std::cout << "test libevent!\n";
    ///创建libevent的上下文

    auto *base = event_base_new();
    if (base)
    {
        std::cout << "event_base_new success!" << std::endl;
    }
    return 0;
}
