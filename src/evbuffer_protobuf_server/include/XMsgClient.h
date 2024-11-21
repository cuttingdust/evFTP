/**
 * @file   XMsgClient.h
 * @brief  
 *
 * Detailed description if necessary.
 *
 * @author 31667
 * @date   2024-11-21
 */

#ifndef XMSGCLIENT_H
#define XMSGCLIENT_H

#include <string>
#include <memory>

class XMsgClient
{
public:
    XMsgClient();
    virtual ~XMsgClient();

public:
    void start();
    void setServerIp(const std::string& ip);
    void setServerPort(int port);

private:
    class PImpl;
    std::shared_ptr<PImpl> impl_;
};

#endif // XMSGCLIENT_H
