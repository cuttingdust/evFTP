#include "XFTPFactory.h"
#include "XFTPServerCMD.h"

auto XFTPFactory::createTask() -> XTask *
{
    XFTPServerCMD *x = new XFTPServerCMD();

    //ע��ftp��Ϣ�������

    return x;
}

XFTPFactory::XFTPFactory()
{
}

XFTPFactory::~XFTPFactory()
{
}
