#include "XMsgHead.pb.h"

#include <iostream>
#include <string>

int main(int argc, char *argv[])
{
    XMsg::XMsgHead head;
    head.set_msgsize(1024);
    head.set_msgstr("===============test for protobuf ============");

    std::cout << "msgsize: " << head.msgsize() << std::endl;
    std::cout << "msgstr: " << head.msgstr() << std::endl;
    std::cin.get();
    return 0;
}
