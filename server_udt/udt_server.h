#ifndef __UDT_SERVER_H_
#define __UDT_SERVER_H_

#include <deque>

#include "send_msg_ctrl.h"

class UDTServer
{
public:
    UDTServer(void);
    void Run(int listen_port);

private:
    int CreateListenSocket(int port);
    int RecvMsg(const UDTSOCKET& sock, bool& bHaveMsgStill);
    void HandleReadFds(const std::set<UDTSOCKET>& readfds);

private:
    UDTSOCKET listen_sock_;
    char udtbuf_[1024 * 100];
    size_t udtbuf_recved_len_;
    int udt_running_;
    int udt_eid_;
    std::shared_ptr<SendMsgCtrl> send_msg_ctrl_ptr_;
};

#endif
