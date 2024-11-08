/**
 * @file   XFTPPort.h
 * @brief  
 *
 * Detailed description if necessary.
 *
 * @author 31667
 * @date   2024-11-07
 */

#ifndef XFTPPORT_H
#define XFTPPORT_H

#include "XFTPTask.h"

class XFTPPort : public XFTPTask
{
public:
    XFTPPort();
    virtual ~XFTPPort();

public:
    auto parse(const std::string &type, const std::string &msg) -> void override;
};

#endif // XFTPPORT_H
