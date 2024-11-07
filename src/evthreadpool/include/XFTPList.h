/**
 * @file   XFTPList.h
 * @brief  
 *
 * Detailed description if necessary.
 *
 * @author 31667
 * @date   2024-11-07
 */

#ifndef XFTPLIST_H
#define XFTPLIST_H
#include "XFtpTask.h"

class XFTPList : public XFTPTask
{
public:
    XFTPList();
    virtual ~XFTPList();

public:
    auto parse(const std::string &type, const std::string &msg) -> void override;
};

#endif // XFTPLIST_H
