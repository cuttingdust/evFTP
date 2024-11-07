#include "XFTPUser.h"
#include <iostream>

XFTPUser::XFTPUser()
{
}

XFTPUser::~XFTPUser()
{
}

auto XFTPUser::init() -> bool
{
    std::cout << "XFTPUser::init" << std::endl;
    return true;
}

auto XFTPUser::parse(const std::string &type, const std::string &msg) -> void
{
    std::cout << "XFTPUser::parse " << type << " " << msg << std::endl;
    resCMD("230 Login successful.\r\n");
}
