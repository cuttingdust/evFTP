/**
 * @file   XFTPServerCMD.h
 * @brief  
 *
 * Detailed description if necessary.
 *
 * @author 31667
 * @date   2024-11-06
 */

#ifndef XFTPSERVERCMD_H
#define XFTPSERVERCMD_H
#include "XTask.h"

class XFTPServerCMD : public XTask
{
public:
    XFTPServerCMD();
    virtual ~XFTPServerCMD();

public:
    auto init() -> bool override;
};

#endif // XFTPSERVERCMD_H
