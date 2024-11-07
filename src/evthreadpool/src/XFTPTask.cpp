#include "XFTPTask.h"

#include <event2/bufferevent.h>
#include <event2/event.h>

auto XFTPTask::setCallback(struct bufferevent *bev) -> void
{
    bufferevent_setcb(bev, readCB, writeCB, eventCB, this);
    bufferevent_enable(bev, EV_READ | EV_WRITE);
}

auto XFTPTask::readCB(bufferevent *bev, void *arg) -> void
{
    XFTPTask *t = (XFTPTask *)arg;
    t->read(bev);
}

auto XFTPTask::writeCB(bufferevent *bev, void *arg) -> void
{
    XFTPTask *t = (XFTPTask *)arg;
    t->write(bev);
}

auto XFTPTask::eventCB(struct bufferevent *bev, short what, void *arg) -> void
{
    XFTPTask *t = (XFTPTask *)arg;
    t->event(bev, what);
}
