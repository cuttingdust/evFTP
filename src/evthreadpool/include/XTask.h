/**
 * @file   XTask.h
 * @brief  
 *
 * Detailed description if necessary.
 *
 * @author 31667
 * @date   2024-11-06
 */

#ifndef XTASK_H
#define XTASK_H

class XTask
{
public:
    virtual auto init() -> bool = 0;

public:
    struct event_base *base_      = 0;
    int                sock_      = 0;
    int                thread_id_ = 0;
};

#endif // XTASK_H
