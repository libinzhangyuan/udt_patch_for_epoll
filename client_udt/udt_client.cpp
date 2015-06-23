#include <arpa/inet.h>
#include <udt.h>
#include <iostream>
#include <cstring>
#include <sys/socket.h>

#include "udt_client.h"

const int g_IP_Version = AF_INET;
//const int g_Socket_Type = SOCK_STREAM;
const int g_Socket_Type = SOCK_DGRAM;


UDTClient::UDTClient(int local_port, const std::string& ip_connect_to, int port_connect_to) :
    sock_(UDT::INVALID_SOCK),
    udtbuf_recved_len_(0),
    udt_running_(0),
    udt_eid_(-1)
{
    Run(local_port, ip_connect_to, port_connect_to);
}


// return -1 means error. otherwise return 0
int UDTClient::CreateSocket(int local_port)
{
    //sock_ = UDT::socket(AF_INET, SOCK_STREAM, 0);
    sock_ = UDT::socket(AF_INET, SOCK_DGRAM, 0);
    if (UDT::INVALID_SOCK == sock_)
    {
        std::cout << "UDT socket: " << UDT::getlasterror().getErrorCode() << ' ' << UDT::getlasterror().getErrorMessage();
        return -1;
    }

    // setopt
    {
        bool block = true;  // 设置成阻塞模式, 然后容易保证写的完整性
        UDT::setsockopt(sock_, 0, UDT_SNDSYN, &block, sizeof(bool));
        UDT::setsockopt(sock_, 0, UDT_RCVSYN, &block, sizeof(bool));

        //UDT::setsockopt(sock_, 0, UDT_MSS, new int(1500), sizeof(int));

        //int snd_buf = 16000;// 8192;
        //int rcv_buf = 16000;//8192;
        //UDT::setsockopt(sock_, 0, UDT_SNDBUF, &snd_buf, sizeof(int)); // use default:10MB
        //UDT::setsockopt(sock_, 0, UDT_RCVBUF, &rcv_buf, sizeof(int)); // use default:10MB


        //int fc = 4096;
        //UDT::setsockopt(sock_, 0, UDT_FC, &fc, sizeof(int));

        bool reuse = true;
        UDT::setsockopt(sock_, 0, UDT_REUSEADDR, &reuse, sizeof(bool));

        bool rendezvous = false;
        UDT::setsockopt(sock_, 0, UDT_RENDEZVOUS, &rendezvous, sizeof(bool));
    }

    // bind
    sockaddr_in my_addr;
    my_addr.sin_family = AF_INET;
    my_addr.sin_port = htons(local_port);
    my_addr.sin_addr.s_addr = INADDR_ANY;
    std::memset(&(my_addr.sin_zero), '\0', 8);
    if (UDT::ERROR == UDT::bind(sock_, (sockaddr*)&my_addr, sizeof(my_addr)))
    {
        std::cout << "UDT bind: " << UDT::getlasterror().getErrorCode() << ' ' << UDT::getlasterror().getErrorMessage() << std::endl;
        return -1;
    }

    // success
	return 0;
}

int UDTClient::SendMsg(const UDTSOCKET& sock, const std::string& msg)
{
    std::cout << "UDT client SendMsg: " << msg << std::endl;

    int send_ret = UDT::sendmsg(sock, msg.c_str(), msg.size());
    if (UDT::ERROR == send_ret)
    {
        std::cout << "UDT sendmsg: " << UDT::getlasterror().getErrorCode() << ' ' << UDT::getlasterror().getErrorMessage() << std::endl;
        return 0;
    }
    if (static_cast<size_t>(send_ret) != msg.size())
    {
        std::cout << "UDT sendmsg: not all msg send!  add lowwatier for send socket is a good thing" << std::endl;
        return 0;
    }
    return 1;
}

int UDTClient::RecvMsg(const UDTSOCKET& sock)
{
    udtbuf_recved_len_ = 0;

    int recv_ret = 0;
    if (UDT::ERROR == (recv_ret = UDT::recvmsg(sock, udtbuf_, sizeof(udtbuf_)))) {
        std::cout << "UDT recv:" << UDT::getlasterror().getErrorMessage() << std::endl;
        if ((CUDTException::EINVPARAM == UDT::getlasterror().getErrorCode()) ||
                (CUDTException::ECONNLOST == UDT::getlasterror().getErrorCode())) {
            udt_running_ = 0;
            UDT::epoll_remove_usock(udt_eid_, sock);
        }
        return 0;
    }

    if (recv_ret > 0) 
    {
        //DEBUG_MSG("UDT Thread Enter.\n");
        udtbuf_recved_len_ = recv_ret;
        std::cout << "UDT recv msg: " << std::string(udtbuf_, udtbuf_recved_len_) << std::endl;
        //DEBUG_MSG(" - UDT Thread Exit.\n");
        return 1;
    }

    std::cout << "UDT recv msg with zero len" << std::endl;
    return 1;
}

int UDTClient::Connect(const std::string& serv_ip, int serv_port)
{
    sockaddr_in serv_addr;
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(serv_port);
    inet_pton(AF_INET, serv_ip.c_str(), &serv_addr.sin_addr);
    std::memset(&(serv_addr.sin_zero), '\0', 8);
    if (UDT::ERROR == UDT::connect(sock_, (sockaddr*)&serv_addr, sizeof(serv_addr)))
    {
        std::cout << "UDT connect: " << UDT::getlasterror().getErrorCode() << ' ' << UDT::getlasterror().getErrorMessage() << std::endl;
        return 0;
    }

    return 1;
}

int UDTClient::Run(int listen_port, const std::string& ip_connect_to, int port_connect_to)
{
	udt_running_ = 0;

    // create, bind
    //
    CreateSocket(listen_port);

    // connect to the server, implict bind
    if (0 == Connect(ip_connect_to, port_connect_to))
    {
        return 0;
    }

    // send hello world for test.
    std::string hello = "hello world!\n";
    if (0 == SendMsg(sock_, hello))
    {
        return 0;
    }

    // epoll
	udt_eid_ = UDT::epoll_create();
	UDT::epoll_add_usock(udt_eid_, sock_);
	
	std::cout << "Run UDT client loop ...\n";
	udt_running_ = 1;


    std::set<UDTSOCKET> readfds;
    std::set<UDTSOCKET> writefds;


	while (udt_running_) {
		int state = UDT::epoll_wait(udt_eid_, &readfds, &writefds, 1, NULL, NULL);
		if (state > 0) {
			for (std::set<UDTSOCKET>::iterator i = readfds.begin(); i != readfds.end(); ++i)
            {
                UDTSOCKET cur_sock = *i;

                if (cur_sock == sock_)
                {
                    RecvMsg(cur_sock);
                    continue;
                }
                else
                {
                    std::cout << "UDT epoll_wait: readfds unknown sock" << std::endl;
                }
			}

            for (std::set<UDTSOCKET>::iterator i = writefds.begin(); i != writefds.end(); ++i)
            {
                UDTSOCKET cur_sock = *i;

                if (cur_sock == sock_)
                {
                    SendMsg(cur_sock, "1234567890");
                    continue;
                }
                else
                {
                    std::cout << "UDT epoll_wait: writefds unknown sock" << std::endl;
                }
            }
		}
		else {
			if ((CUDTException::EINVPARAM == UDT::getlasterror().getErrorCode()) ||
				(CUDTException::ECONNLOST == UDT::getlasterror().getErrorCode())) {
				udt_running_ = 0;
				//UDT::epoll_remove_usock(eid, cur_sock);
			}
		}
	}

	std::cout << "release UDT epoll ..." << std::endl;
	int release_state = UDT::epoll_release(udt_eid_);
    if (release_state != 0)
        std::cout << "UDT epoll_release: " << release_state << std::endl;
	

	std::cout << "Close client ...";
	int close_state = UDT::close(sock_);
    if (close_state != 0)
        std::cout << "UDT close: " << UDT::getlasterror().getErrorCode() << ' ' << UDT::getlasterror().getErrorMessage() << std::endl;

	std::cout << "ok\n";

	pthread_detach(pthread_self());

    return 1;
}
