/**
 * @file   XFTPStor.h
 * @brief  
 *
 * Detailed description if necessary.
 *
 * @author 31667
 * @date   2024-11-08
 */

#ifndef XFTPSTOR_H
#define XFTPSTOR_H

#include "XFTPTask.h"

class XFTPStor : public XFTPTask
{
public:
    auto parse(const std::string &type, const std::string &msg) -> void override;
    auto read(struct bufferevent *bev) -> void override;
    auto event(struct bufferevent *bev, short what) -> void override;

private:
    FILE *fp_ = nullptr;
    char  buf[1024]{};
};

#endif // XFTPSTOR_H
