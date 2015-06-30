#include <iostream>
#include <cstring>
#include <assert.h>
//#include <sys/socket.h>
//#include <sys/time.h>

#include <udt.h>

#include "send_msg_ctrl.h"

static int event_with_write = UDT_EPOLL_IN | UDT_EPOLL_ERR | UDT_EPOLL_OUT;
static int event_only_read = UDT_EPOLL_IN | UDT_EPOLL_ERR;

void SendMsgCtrl::TrySendMsg(const std::set<UDTSOCKET>& writefds)
{
    for (const UDTSOCKET& sock : writefds)
    {
        TrySendMsgOfOneSock(sock);
    }
}

void SendMsgCtrl::TrySendMsgOfOneSock(const UDTSOCKET& sock)
{
    while (true)
    {
        msg_ptr_t msg = send_msg_buf_.PeekFrontMsg(sock);

        // no more msg need send.
        if (msg == NULL)
        {
            TryRemoveWriteEvent(sock);
            return;
        }

        // do send
        {
            int send_ret = UDT::sendmsg(sock, msg->c_str(), msg->size(), -1, true);
            if (UDT::ERROR == send_ret)
            {
                CUDTException& lasterror = UDT::getlasterror();
                const int error_code = lasterror.getErrorCode();

                // no buffer space is available.
                if (error_code == CUDTException::EASYNCSND)
                {
                    std::cerr << "N"; std::cerr.flush();
                    return; // try send at next epoll return
                }

                // badly case.  do disconnect thing. need remove socket from epoll.
                HandleSendMsgError(sock, lasterror);
                return;
            }

            // send succ, pop this msg from queue, then continue sending next msg.
            assert(static_cast<size_t>(send_ret) == msg->size());
            send_msg_buf_.PopMsg(sock);
            continue;
        }
    }
}

void SendMsgCtrl::HandleSendMsgError(const UDTSOCKET& sock, CUDTException& lasterror)
{
    const int error_code = lasterror.getErrorCode();
    std::cout << "UDT sendmsg disconnect: " << error_code << ' ' << lasterror.getErrorMessage() << std::endl;

    UDT::epoll_remove_usock(udt_eid_, sock);
    CleanMsg(sock);
    events_with_write_.erase(sock);
}

void SendMsgCtrl::AddMsgToSendBuf(const UDTSOCKET& sock, const msg_ptr_t& msg)
{
    send_msg_buf_.AddMsg(sock, msg);
    if (!IsWriteEventAdd(sock))
        AddWriteEvent(sock);
}

void SendMsgCtrl::CleanAll(void)
{
}

void SendMsgCtrl::CleanMsg(const UDTSOCKET& sock)
{
    send_msg_buf_.CleanMsg(sock);
}


bool SendMsgCtrl::IsWriteEventAdd(const UDTSOCKET& sock) const
{
    return events_with_write_.find(sock) != events_with_write_.end();
}

void SendMsgCtrl::AddWriteEvent(const UDTSOCKET& sock)
{
    events_with_write_.insert(sock);
    std::cerr << "A"; std::cerr.flush();
    UDT::epoll_add_usock(udt_eid_, sock, &event_with_write);
}

void SendMsgCtrl::TryRemoveWriteEvent(const UDTSOCKET& sock)
{
    if (IsWriteEventAdd(sock))
    {
        events_with_write_.erase(sock);
        std::cerr << "R"; std::cerr.flush();
        UDT::epoll_remove_usock(udt_eid_, sock);
        UDT::epoll_add_usock(udt_eid_, sock, &event_only_read);
    }
}
