/**
 * @file   XFTPRetr.h
 * @brief  
 *
 * Detailed description if necessary.
 *
 * @author 31667
 * @date   2024-11-08
 */

#ifndef XFTPRETR_H
#define XFTPRETR_H

#include "XFTPTask.h"

class XFTPRetr : public XFTPTask
{
public:
    XFTPRetr();
    virtual ~XFTPRetr();

public:
    auto parse(const std::string &type, const std::string &msg) -> void override;
    auto write(struct bufferevent *bev) -> void override;
    auto event(struct bufferevent *bev, short what) -> void override;
};

#endif // XFTPRETR_H
