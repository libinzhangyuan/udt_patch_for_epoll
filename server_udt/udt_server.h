#ifndef __UDT_SERVER_H_
#define __UDT_SERVER_H_

class UDTServer
{
public:
    UDTServer(void);
    void Run(int listen_port);

private:
    int CreateListenSocket(int port);
    int SendMsg(const UDTSOCKET& sock, const std::string& msg);

    // return 1 means recv a package.
    //   at this situation:   bHaveMsgStill == true, means have another package need recv.
    //                        bHaveMsgStill == false means do not have another package need recv.
    // return 0 means do not recv a package.
    int RecvMsg(const UDTSOCKET& sock, int count_of_event, bool& bHaveMsgStill);

private:
    UDTSOCKET listen_sock_;
    char udtbuf_[1024 * 100];
    size_t udtbuf_recved_len_;
    int udt_running_;
    int udt_eid_;
};

#endif
