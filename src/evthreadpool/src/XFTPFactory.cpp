#include "XFTPFactory.h"

#include "XFTPServerCMD.h"
#include "XFTPUser.h"
#include "XFTPList.h"
#include "XFTPPort.h"
#include "XFTPRetr.h"
#include "XFTPStor.h"

auto XFTPFactory::createTask() -> XTask *
{
    XFTPServerCMD *x = new XFTPServerCMD();

    /// 注册ftp消息处理对象
    x->reg("USER", new XFTPUser());

    XFTPList *list = new XFTPList;
    x->reg("PWD", list);

    x->reg("PORT", new XFTPPort);

    x->reg("LIST", list);

    x->reg("CWD", list);

    x->reg("CDUP", list);

    x->reg("RETR", new XFTPRetr);

    x->reg("STOR", new XFTPStor);
    return x;
}

XFTPFactory::XFTPFactory()
{
}

XFTPFactory::~XFTPFactory()
{
}
