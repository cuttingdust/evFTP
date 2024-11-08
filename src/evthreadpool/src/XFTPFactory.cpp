#include "XFTPFactory.h"

#include "XFTPServerCMD.h"
#include "XFTPUser.h"
#include "XFTPList.h"
#include "XFTPPort.h"

auto XFTPFactory::createTask() -> XTask *
{
    XFTPServerCMD *x = new XFTPServerCMD();

    /// 注册ftp消息处理对象
    x->reg("USER", new XFTPUser());

    XFTPList *list = new XFTPList;
    x->reg("PWD", list);

    x->reg("PORT", new XFTPPort);

    x->reg("LIST", list);


    return x;
}

XFTPFactory::XFTPFactory()
{
}

XFTPFactory::~XFTPFactory()
{
}
