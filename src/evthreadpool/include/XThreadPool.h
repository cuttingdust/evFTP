/**
 * @file   XThreadPool.h
 * @brief  线程池 
 *
 * Detailed description if necessary.
 *
 * @author 31667
 * @date   2024-11-05
 */

#ifndef XTHREADPOOL_H
#define XTHREADPOOL_H

#include <memory>

class XTask;

class XThreadPool
{
public:
    static XThreadPool *getInstance()
    {
        static XThreadPool pool;
        return &pool;
    }

    /// \brief 初始化所有线程并启动线程
    /// \param threadNum
    auto init(int threadNum = 4) -> void;

    /// \brief 分发线程
    /// \param task
    auto dispatch(XTask *task) -> void;

private:
    XThreadPool();
    virtual ~XThreadPool();

private:
    class PImpl;
    std::shared_ptr<PImpl> impl_;
};

#endif // XTHREADPOOL_H
