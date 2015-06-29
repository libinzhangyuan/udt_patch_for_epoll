#include <arpa/inet.h>
#include <udt.h>
#include <iostream>
#include <cstring>
#include <sys/socket.h>
#include <memory>
#include <algorithm>

#include "udt_client.h"
#include "test_util.h"


const int g_IP_Version = AF_INET;
//const int g_Socket_Type = SOCK_STREAM;
const int g_Socket_Type = SOCK_DGRAM;



UDTClient::UDTClient(int local_port, const std::string& ip_connect_to, int port_connect_to, recv_callback_func_t recv_func) :
    recv_callback_func_(recv_func),
    local_port_(local_port),
    ip_connect_to_(ip_connect_to),
    port_connect_to_(port_connect_to),
    sock_(UDT::INVALID_SOCK),
    udtbuf_recved_len_(0),
    udt_running_(0),
    udt_eid_(-1)
{
}

int UDTClient::Start(void)
{
    udt_running_ = 0;

    // create, bind
    //
    CreateSocket();

    // connect to the server, implict bind
    if (0 == Connect())
    {
        return 0;
    }

    udt_thread_ = std::thread(std::bind(&UDTClient::Run, shared_from_this()));
    return 1;
}

void UDTClient::SendMsg(msg_ptr_t msg)
{
    send_msg_queue_.push(msg);
}

size_t UDTClient::SendMsgQueueSize(void)
{
    return send_msg_queue_.size();
}

std::deque<msg_ptr_t> UDTClient::RecvMsg(void) // todo: using move symatic
{
    return std::deque<msg_ptr_t>();
}

// return -1 means error. otherwise return 0
int UDTClient::CreateSocket(void)
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
        int snd_buf = 14600;// 1460 * 10
        int rcv_buf = 93440 * 10;
        UDT::setsockopt(sock_, 0, UDT_SNDBUF, &snd_buf, sizeof(int));
        UDT::setsockopt(sock_, 0, UDT_RCVBUF, &rcv_buf, sizeof(int)); 


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
    my_addr.sin_port = htons(local_port_);
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

void UDTClient::TrySendMsg(void)
{
    //std::queue<msg_ptr_t>&& msgs_need_send = send_msg_queue_.grab_all(); // using right refrence because we want to change it.
    std::queue<msg_ptr_t> msgs_need_send = send_msg_queue_.grab_all();

    while (!msgs_need_send.empty())
    {
        msg_ptr_t msg = msgs_need_send.front();
        msgs_need_send.pop();

        uint64_t begin_time = iclock64();
        DoSendOneMsg(*msg);
        uint64_t end_time = iclock64();
        /*if (end_time - begin_time > 10)
            std::cout << "send time too long: " << end_time - begin_time << std::endl;
            */
        if (end_time - begin_time > 10)
            std::cerr << "S";
        else
            std::cerr << "s";
        std::cerr.flush();
    }
}

void UDTClient::DoSendOneMsg(const std::string& msg)
{
    //std::cout << "UDT client DoSendMsg: " << msg << std::endl;

    int send_ret = UDT::sendmsg(sock_, msg.c_str(), msg.size());
    if (UDT::ERROR == send_ret)
    {
        std::cout << "UDT sendmsg: " << UDT::getlasterror().getErrorCode() << ' ' << UDT::getlasterror().getErrorMessage() << std::endl;
        if (UDT::getlasterror().getErrorCode() == CUDTException::ECONNLOST)
        {
            udt_running_ = 0;
        }
        return;
    }
    if (static_cast<size_t>(send_ret) != msg.size())
    {
        std::cout << "UDT sendmsg: not all msg send!" << std::endl;
        return;
    }
}

void UDTClient::DoRecvMsg(const UDTSOCKET& sock, bool& bHaveMsgStill)
{
    udtbuf_recved_len_ = 0;

    int recv_ret = 0;
    if (UDT::ERROR == (recv_ret = UDT::recvmsg(sock, udtbuf_, sizeof(udtbuf_), &bHaveMsgStill))) {
        CUDTException& lasterror = UDT::getlasterror();
        int error_code = lasterror.getErrorCode();

        std::cout << "UDT recv: " << error_code << " " << lasterror.getErrorMessage() << std::endl;
        if (CUDTException::EINVPARAM == error_code || CUDTException::ECONNLOST == error_code || CUDTException::EINVSOCK == error_code) {
            udt_running_ = 0;
            UDT::epoll_remove_usock(udt_eid_, sock);
        }
        return;
    }

    if (recv_ret > 0) 
    {
        //DEBUG_MSG("UDT Thread Enter.\n");
        udtbuf_recved_len_ = recv_ret;
        if (recv_callback_func_)
        {
            std::shared_ptr<std::string> recved_str(new std::string(udtbuf_, udtbuf_recved_len_));
            recv_callback_func_(recved_str);
        }
        //DEBUG_MSG(" - UDT Thread Exit.\n");
        return;
    }

    std::cout << "UDT recv msg with zero len" << std::endl;
    return;
}

int UDTClient::Connect(void)
{
    sockaddr_in serv_addr;
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(port_connect_to_);
    inet_pton(AF_INET, ip_connect_to_.c_str(), &serv_addr.sin_addr);
    std::memset(&(serv_addr.sin_zero), '\0', 8);
    std::cout << "connect to: " << ip_connect_to_ << " " << port_connect_to_ << std::endl;
    if (UDT::ERROR == UDT::connect(sock_, (sockaddr*)&serv_addr, sizeof(serv_addr)))
    {
        std::cout << "UDT connect: " << UDT::getlasterror().getErrorCode() << ' ' << UDT::getlasterror().getErrorMessage() << std::endl;
        return 0;
    }

    return 1;
}

void UDTClient::TryRecvMsg(const std::set<UDTSOCKET>& readfds)
{
    for (std::set<UDTSOCKET>::iterator i = readfds.begin(); i != readfds.end(); ++i)
    {
        UDTSOCKET cur_sock = *i;

        if (cur_sock == sock_)
        {
            bool bHaveMsgStill = false;
            do
            {
                bHaveMsgStill = false;
                DoRecvMsg(cur_sock, bHaveMsgStill);
            } while (bHaveMsgStill);
        }
        else
        {
            std::cout << "UDT epoll_wait: readfds unknown sock" << std::endl;
        }
    }
}

int UDTClient::Run(void)
{
	udt_running_ = 1;

    // epoll
	udt_eid_ = UDT::epoll_create();
    int read_event = UDT_EPOLL_IN | UDT_EPOLL_ERR;
	UDT::epoll_add_usock(udt_eid_, sock_, &read_event);
	
	std::cout << "Run UDT client loop ...\n";
	udt_running_ = 1;

	while (udt_running_) {
        std::set<UDTSOCKET> readfds;

        int state = UDT::epoll_wait(udt_eid_, &readfds, NULL, 1, NULL, NULL);
        if (state < 0) {
            CUDTException& lasterror = UDT::getlasterror();
            int error_code = lasterror.getErrorCode();
            std::cout << "UDT epoll_wait: " << error_code << ' ' << lasterror.getErrorMessage() << std::endl;
			if ((CUDTException::EINVPARAM == error_code) ||
				(CUDTException::ECONNLOST == error_code)) {
				udt_running_ = 0;
				//UDT::epoll_remove_usock(eid, cur_sock);
			}
            continue;
        }

        // send msg first.
        if (state == 0)
        {
            std::cerr << ".";
            std::cerr.flush();
        }
        TrySendMsg();

		if (state > 0) {
            // recv msg
            TryRecvMsg(readfds);
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
