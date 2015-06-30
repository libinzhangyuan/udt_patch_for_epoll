#include <arpa/inet.h>
#include <udt.h>
#include <iostream>
#include <cstring>
#include <sys/socket.h>
#include <sys/time.h>


#include "test_util.h"
#include "udt_server.h"

const int g_IP_Version = AF_INET;
//const int g_Socket_Type = SOCK_STREAM;
const int g_Socket_Type = SOCK_DGRAM;

static int read_event = UDT_EPOLL_IN | UDT_EPOLL_ERR;

/* get system time */
static inline void itimeofday(long *sec, long *usec)
{
    struct timeval time;
    gettimeofday(&time, NULL);
    if (sec) *sec = time.tv_sec;
    if (usec) *usec = time.tv_usec;
}

/* get clock in millisecond 64 */
static inline uint64_t iclock64(void)
{
    long s, u;
    uint64_t value;
    itimeofday(&s, &u);
    value = ((uint64_t)s) * 1000 + (u / 1000);
    return value;
}



UDTServer::UDTServer(void) :
    listen_sock_(UDT::INVALID_SOCK),
    udtbuf_recved_len_(0),
    udt_running_(0),
    udt_eid_(-1)
{
}

// return -1 means error. otherwise return 0
int UDTServer::CreateListenSocket(int listen_port)
{
    //listen_sock_ = UDT::socket(AF_INET, SOCK_STREAM, 0);
    listen_sock_ = UDT::socket(AF_INET, SOCK_DGRAM, 0);
    if (UDT::INVALID_SOCK == listen_sock_)
    {
        std::cout << "UDT socket: " << UDT::getlasterror().getErrorCode() << ' ' << UDT::getlasterror().getErrorMessage();
        return -1;
    }

    // setopt
    {
        bool block = false;
        UDT::setsockopt(listen_sock_, 0, UDT_SNDSYN, &block, sizeof(bool));
        UDT::setsockopt(listen_sock_, 0, UDT_RCVSYN, &block, sizeof(bool));

        //UDT::setsockopt(listen_sock_, 0, UDT_MSS, new int(1500), sizeof(int)); // default is 1500

        int snd_buf = 93440;// 1460 * 64 = 93440
        int rcv_buf = 93440;
        UDT::setsockopt(listen_sock_, 0, UDT_SNDBUF, &snd_buf, sizeof(int)); // default is 10MB
        UDT::setsockopt(listen_sock_, 0, UDT_RCVBUF, &rcv_buf, sizeof(int)); // default is 10MB


        //int fc = 4096;
        //UDT::setsockopt(listen_sock_, 0, UDT_FC, &fc, sizeof(int));

        bool reuse = true;
        UDT::setsockopt(listen_sock_, 0, UDT_REUSEADDR, &reuse, sizeof(bool));

        bool rendezvous = false;
        UDT::setsockopt(listen_sock_, 0, UDT_RENDEZVOUS, &rendezvous, sizeof(bool));
    }

    // bind
    sockaddr_in my_addr;
    my_addr.sin_family = AF_INET;
    my_addr.sin_port = htons(listen_port);
    my_addr.sin_addr.s_addr = INADDR_ANY;
    std::memset(&(my_addr.sin_zero), '\0', 8);
    if (UDT::ERROR == UDT::bind(listen_sock_, (sockaddr*)&my_addr, sizeof(my_addr)))
    {
        std::cout << "UDT bind: " << UDT::getlasterror().getErrorCode() << ' ' << UDT::getlasterror().getErrorMessage() << std::endl;
        return -1;
    }

    // listen
    //
    if (UDT::ERROR == UDT::listen(listen_sock_, 20))
    {
        std::cout << "UDT listen: " << UDT::getlasterror().getErrorCode() << ' ' << UDT::getlasterror().getErrorMessage() << std::endl;
        return -1;
    }

    // success
	return 0;
}

int UDTServer::RecvMsg(const UDTSOCKET& sock, bool& bHaveMsgStill)
{
    static size_t static_good_recv_count = 0;
    static size_t static_recv_count = 0;
    static_recv_count++;

    udtbuf_recved_len_ = 0;
    bHaveMsgStill = false;

    uint64_t begin_time = iclock64();
    int recv_ret = UDT::recvmsg(sock, udtbuf_, sizeof(udtbuf_), &bHaveMsgStill);
    uint64_t end_time = iclock64();
    if ((end_time - begin_time) > 10)
        std::cout << "\nrecv block:" << end_time - begin_time << std::endl;
    if (UDT::ERROR == recv_ret) {

        CUDTException& lasterror = UDT::getlasterror();
        int error_code = lasterror.getErrorCode();

        // no data available for read.  try read after next epoll wait.
        if (error_code == CUDTException::EASYNCRCV)
            return 0;

        std::cout << "UDT recv: " << error_code << " " <<  lasterror.getErrorMessage() << std::endl;

        if (error_code == CUDTException::EINVPARAM) {
            udt_running_ = 0;
            UDT::epoll_remove_usock(udt_eid_, sock);
        }
        else if (error_code == CUDTException::ECONNLOST || error_code == CUDTException::EINVSOCK) {
            std::cout << "\nepoll_remove_usock" << std::endl;
            UDT::epoll_remove_usock(udt_eid_, sock);
        }
        else {
            std::cout << "RecvMsg: an error do not handle!" << std::endl;
        }
        return 0;
    }

    if (recv_ret > 0) 
    {
        //DEBUG_MSG("UDT Thread Enter.\n");
        udtbuf_recved_len_ = recv_ret;


        // log
        {
            static_good_recv_count++;
            if (static_recv_count % 10 == 0)
            {
                std::cout << static_good_recv_count << '\\' << static_recv_count << "\t";
                std::cout.flush();
                if (static_recv_count % 100 == 0)
                    std::cout << std::endl;
            }
        }
        //DEBUG_MSG(" - UDT Thread Exit.\n");
        return 1;
    }

    std::cout << "UDT recv msg with zero len" << std::endl;
    return 1;
}

void UDTServer::HandleReadFds(const std::set<UDTSOCKET>& readfds)
{
    for (const UDTSOCKET cur_sock : readfds)
    {
        if (cur_sock == listen_sock_)
        {
            std::cout << "accept a new connection!" << std::endl;
            sockaddr addr;
            int addr_len;
            UDTSOCKET new_sock = UDT::accept(listen_sock_, &addr, &addr_len);
            if (new_sock == UDT::INVALID_SOCK)
            {
                std::cout << "UDT accept:" << UDT::getlasterror().getErrorCode() << ' ' << UDT::getlasterror().getErrorMessage() << std::endl;
                continue;
            }

            // add to readfds
            {
                int add_usock_ret = UDT::epoll_add_usock(udt_eid_, new_sock, &read_event);
                if (add_usock_ret < 0)
                    std::cout << "UDT::epoll_add_usock new_sock add error: " << add_usock_ret << std::endl;
            }

            continue;
        }

        // client sock
        {
            bool bHaveMsgStill = false;
            do
            {
                bHaveMsgStill = false;
                RecvMsg(cur_sock, bHaveMsgStill);
                if (udtbuf_recved_len_ > 0)
                {
                    send_msg_ctrl_ptr_->AddMsgToSendBuf(cur_sock, std::make_shared<std::string>(udtbuf_, udtbuf_recved_len_));
                }
            } while (bHaveMsgStill);
        }
    }
}

void UDTServer::Run(int listen_port)
{
	udt_running_ = 0;

    // create, bind and listen
    //
	CreateListenSocket(listen_port);


    // epoll
	udt_eid_ = UDT::epoll_create();

    {
        int add_usock_ret = UDT::epoll_add_usock(udt_eid_, listen_sock_, &read_event);
        if (add_usock_ret < 0)
            std::cout << "UDT::epoll_add_usock error: " << add_usock_ret << std::endl;
    }

    send_msg_ctrl_ptr_.reset(new SendMsgCtrl(udt_eid_));

	std::cout << "Run UDT server loop ...\n";
	udt_running_ = 1;

	while (udt_running_) {
        std::set<UDTSOCKET> readfds;
        std::set<UDTSOCKET> writefds;
		int state = UDT::epoll_wait(udt_eid_, &readfds, &writefds, 10, NULL, NULL);
		if (state > 0) {
            // read
            HandleReadFds(readfds);
            // write
            send_msg_ctrl_ptr_->TrySendMsg(writefds);
		}
        else if (state == 0) {
            std::cout << ".";
            std::cout.flush();
        }
		else {
            std::cout << "UDT epoll_wait: " << UDT::getlasterror().getErrorCode() <<
                    ' ' << UDT::getlasterror().getErrorMessage() << std::endl;
			if ((CUDTException::EINVPARAM == UDT::getlasterror().getErrorCode()) ||
				(CUDTException::ECONNLOST == UDT::getlasterror().getErrorCode())) {
				udt_running_ = 0;
				//UDT::epoll_remove_usock(eid, cur_sock);
			}
		}
	}


    send_msg_ctrl_ptr_->CleanAll();

	std::cout << "release UDT epoll ..." << std::endl;
	int release_state = UDT::epoll_release(udt_eid_);
    if (release_state != 0)
        std::cout << "UDT epoll_release: " << release_state << std::endl;
	

	std::cout << "Close server ...";
	int close_state = UDT::close(listen_sock_);
    if (close_state != 0)
        std::cout << "UDT close:" << UDT::getlasterror().getErrorCode() << ' ' << UDT::getlasterror().getErrorMessage() << std::endl;

	std::cout << "ok\n";

	pthread_detach(pthread_self());
}
