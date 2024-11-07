/**
 * @file   XFTPUser.h
 * @brief  
 *
 * Detailed description if necessary.
 *
 * @author 31667
 * @date   2024-11-07
 */

#ifndef XFTPUSER_H
#define XFTPUSER_H

#include "XFtpTask.h"

class XFTPUser : public XFTPTask
{
public:
    XFTPUser();
    virtual ~XFTPUser();

public:
    auto init() -> bool override;

    auto parse(const std::string &type, const std::string &msg) -> void override;
};

#endif // XFTPUSER_H
