//#include <iostream>
//#include <cstring>
//#include <sys/socket.h>
//#include <sys/time.h>

//#include "test_util.h"
#include "send_msg_buf.h"


const std::queue<msg_ptr_t>& SendMsgBuf::GetMsgQueue(const UDTSOCKET& sock) const
{
    static std::queue<msg_ptr_t> empty_msg_buf;
    auto iter = send_msg_buff_.find(sock);
    if (iter == send_msg_buff_.end())
        return empty_msg_buf;
    return iter->second;
}

bool SendMsgBuf::HasMsg(const UDTSOCKET& sock) const
{
    auto iter = send_msg_buff_.find(sock);
    if (iter != send_msg_buff_.end())
        return iter->second.size() > 0;
    return false;
}

msg_ptr_t SendMsgBuf::PeekFrontMsg(const UDTSOCKET& sock) const
{
    auto iter = send_msg_buff_.find(sock);
    if (iter != send_msg_buff_.end() && iter->second.size() > 0)
        return iter->second.front();
    return msg_ptr_t();
}

void SendMsgBuf::PopMsg(const UDTSOCKET& sock)
{
    auto iter = send_msg_buff_.find(sock);
    if (iter != send_msg_buff_.end())
    {
        if (iter->second.size() > 0)
            iter->second.pop();
        if (iter->second.size() == 0)
            send_msg_buff_.erase(iter);
    }
}

void SendMsgBuf::AddMsg(const UDTSOCKET& sock, const msg_ptr_t& ptr)
{
    auto iter = send_msg_buff_.find(sock);
    if (iter != send_msg_buff_.end())
        iter->second.push(ptr);
    else
    {
        std::queue<msg_ptr_t> msg_queue;
        msg_queue.push(ptr);
        send_msg_buff_[sock] = msg_queue;
    }
}

void SendMsgBuf::CleanMsg(const UDTSOCKET& sock)
{
    auto iter = send_msg_buff_.find(sock);
    if (iter != send_msg_buff_.end())
        send_msg_buff_.erase(iter);
}

void SendMsgBuf::CleanAllMsg(void)
{
    send_msg_buff_.clear();
}
