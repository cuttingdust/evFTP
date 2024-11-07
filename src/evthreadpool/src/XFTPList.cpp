#include "XFTPList.h"

XFTPList::XFTPList()
{
}

XFTPList::~XFTPList()
{
}

auto XFTPList::parse(const std::string &type, const std::string &msg) -> void
{
    std::string resmsg = "";
    if (type == "PWD")
    {
        /// 257 "/" is current directory.
        resmsg = "257 \"";
        resmsg += cmdTask_->curDir_;
        resmsg += "\" is current dir.\r\n";

        resCMD(resmsg);
    }
}
