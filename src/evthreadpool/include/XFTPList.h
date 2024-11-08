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
    auto write(struct bufferevent *bev) -> void override;
    auto event(struct bufferevent *bev, short what) -> void override;

public:
    std::string getListData(const std::string &path);
};

#endif // XFTPLIST_H
