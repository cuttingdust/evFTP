/**
 * @file   XFTPFactory.h
 * @brief  
 *
 * Detailed description if necessary.
 *
 * @author 31667
 * @date   2024-11-07
 */

#ifndef XFTPFACTORY_H
#define XFTPFACTORY_H

#include "XTask.h"

class XFTPFactory
{
public:
    static XFTPFactory *getInstance()
    {
        static XFTPFactory instance;
        return &instance;
    }

public:
    auto createTask() -> XTask *;

private:
    XFTPFactory();
    virtual ~XFTPFactory();
};

#endif // XFTPFACTORY_H
