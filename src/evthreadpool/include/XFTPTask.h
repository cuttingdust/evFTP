/**
 * @file   XFTPTask.h
 * @brief  
 *
 * Detailed description if necessary.
 *
 * @author 31667
 * @date   2024-11-07
 */

#ifndef XFTPTASK_H
#define XFTPTASK_H

#include "XTask.h"

class XFTPTask : public XTask
{
public:
    virtual auto read(struct bufferevent *bev) -> void{};
    virtual auto write(struct bufferevent *bev) -> void{};
    virtual auto event(struct bufferevent *bev, short what) -> void{};
    auto         setCallback(struct bufferevent *bev) -> void;

protected:
    static auto readCB(bufferevent *bev, void *arg) -> void;
    static auto writeCB(bufferevent *bev, void *arg) -> void;
    static auto eventCB(struct bufferevent *bev, short what, void *arg) -> void;
};

#endif // XFTPTASK_H
