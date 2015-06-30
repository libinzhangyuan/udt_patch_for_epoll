#ifndef __UDT_SEND_MSG_BUF_H_
#define __UDT_SEND_MSG_BUF_H_

#include "udt_epoll_def.h"
#include <queue>
#include <map>

class SendMsgBuf
{
public:
    SendMsgBuf() {};

    const std::queue<msg_ptr_t>& GetMsgQueue(const UDTSOCKET& sock) const;

    bool HasMsg(const UDTSOCKET& sock) const;

    // only peek one msg. not remove it.
    // if there is no msg. return empty msg_ptr_t.
    msg_ptr_t PeekFrontMsg(const UDTSOCKET& sock) const;

    // remove one msg at front.
    void PopMsg(const UDTSOCKET& sock);

    void AddMsg(const UDTSOCKET& sock, const msg_ptr_t& ptr);

    void CleanMsg(const UDTSOCKET& sock);

    void CleanAllMsg(void);

private:

private:
    std::map<UDTSOCKET, std::queue<msg_ptr_t> > send_msg_buff_;
};

#endif // __UDT_SEND_MSG_BUF_H_
