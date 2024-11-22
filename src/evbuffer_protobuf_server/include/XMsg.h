/**
 * @file   XMsg.h
 * @brief  
 *
 * Detailed description if necessary.
 *
 * @author 31667
 * @date   2024-11-22
 */

#ifndef XMSG_H
#define XMSG_H

#include <cstring>
#define MAX_MSG_SIZE 8192 /// ͷ����Ϣ������ֽ���


/// ���еĺ���������
class XMessage
{
public:
    int   size     = 0; ///< ���ݴ�С
    char *data     = 0; ///< ���ݴ�ţ�protobuf�����л�������ݣ�
    int   recvSize = 0; ///< �Ѿ����յ����ݴ�С
public:
    bool alloc(int s)
    {
        if (s <= 0 || s > MAX_MSG_SIZE)
            return false;
        if (data)
            delete data;
        data = new char[s];
        if (!data)
            return false;
        this->size     = s;
        this->recvSize = 0;
        return true;
    }

    /// �ж������Ƿ�������
    bool recved()
    {
        if (size <= 0)
            return false;
        return (recvSize == size);
    }

    void clear()
    {
        delete data;
        memset(this, 0, sizeof(XMessage));
    }
};

#endif // XMSG_H