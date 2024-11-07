/**
 * @file   XFTPServerCMD.h
 * @brief  
 *
 * Detailed description if necessary.
 *
 * @author 31667
 * @date   2024-11-06
 */

#ifndef XFTPSERVERCMD_H
#define XFTPSERVERCMD_H
#include "XFTPTask.h"

class XFTPServerCMD : public XFTPTask
{
public:
    XFTPServerCMD();
    virtual ~XFTPServerCMD();

public:
    auto init() -> bool override;
    auto read(struct bufferevent *bev) -> void override;
    auto event(struct bufferevent *bev, short what) -> void override;
};

#endif // XFTPSERVERCMD_H
