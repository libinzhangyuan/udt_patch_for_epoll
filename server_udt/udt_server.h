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
    int RecvMsg(const UDTSOCKET& sock);

private:
    UDTSOCKET listen_sock_;
    char udtbuf_[1024 * 100];
    size_t udtbuf_recved_len_;
    int udt_running_;
    int udt_eid_;
};

#endif
