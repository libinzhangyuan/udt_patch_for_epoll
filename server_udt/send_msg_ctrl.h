#ifndef __UDT_SEND_MSG_CTRL_H_
#define __UDT_SEND_MSG_CTRL_H_

#include <set>

#include "send_msg_buf.h"

class CUDTException;

class SendMsgCtrl
{
public:
    SendMsgCtrl(int udt_eid) : udt_eid_(udt_eid) {};

    void TrySendMsg(const std::set<UDTSOCKET>& writefds);
    void AddMsgToSendBuf(const UDTSOCKET& sock, const msg_ptr_t& msg);
    void CleanAll(void);
    void CleanMsg(const UDTSOCKET& sock);

private:
    void TrySendMsgOfOneSock(const UDTSOCKET& sock);
    void HandleSendMsgError(const UDTSOCKET& sock, CUDTException& lasterror);

    bool IsWriteEventAdd(const UDTSOCKET& sock) const;
    void AddWriteEvent(const UDTSOCKET& sock);
    void TryRemoveWriteEvent(const UDTSOCKET& sock);

private:
    int udt_eid_;
    std::set<UDTSOCKET> events_with_write_; // Recording the udtsockets that is added to write event.
    SendMsgBuf send_msg_buf_;
};

#endif // __UDT_SEND_MSG_CTRL_H_
