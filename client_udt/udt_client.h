#ifndef __UDT_CLIENT_H_
#define __UDT_CLIENT_H_

typedef int UDTSOCKET;

class UDTClient
{
public:
    UDTClient(int local_port, const std::string& ip_connect_to, int port_connect_to);
    //void SendMsg(const std::string& msg);

private:
    int SendMsg(const UDTSOCKET& sock, const std::string& msg);
    int RecvMsg(const UDTSOCKET& sock);
    int Run(int listen_port, const std::string& ip_connect_to, int port_connect_to);
    int CreateSocket(int local_port);
    int Connect(const std::string& server_ip, int server_port);

private:
    UDTSOCKET sock_;
    char udtbuf_[1024 * 100];
    size_t udtbuf_recved_len_;
    int udt_running_;
    int udt_eid_;
};

#endif
