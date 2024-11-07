#include "XFTPFactory.h"
#include "XFTPServerCMD.h"

auto XFTPFactory::createTask() -> XTask *
{
    XFTPServerCMD *x = new XFTPServerCMD();

    //注册ftp消息处理对象

    return x;
}

XFTPFactory::XFTPFactory()
{
}

XFTPFactory::~XFTPFactory()
{
}
