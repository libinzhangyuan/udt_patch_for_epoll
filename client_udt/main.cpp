#include <iostream>
#include <cstring>
#include <signal.h>
#include <cstdlib>
#include <sys/time.h>
#include <thread>
#include <chrono>
#include <sstream>

#include "test_util.h"
#include "udt_client.h"

#define CLOCK_INTERVAL_STR "haha"
msg_ptr_t make_test_str(size_t test_str_size)
{
    std::ostringstream ostr;
    ostr << iclock64();
    std::string msg_str = ostr.str();
    msg_str += test_str(CLOCK_INTERVAL_STR, test_str_size - msg_str.size());
    return msg_ptr_t(new std::string(msg_str));
}

uint64_t get_time_from_msg(const std::string& msg)
{
    std::size_t pos = msg.find(CLOCK_INTERVAL_STR);
    if (pos == std::string::npos)
    {
        std::cout << "wrong msg: " << msg << std::endl;
        return 0;
    }
    const std::string& time_str = msg.substr(0, pos);
    return std::atoll(time_str.c_str());
}

void recv_func(const std::shared_ptr<std::string>& str)
{
    //std::cout << "recv: " << *str << std::endl;
    uint64_t send_time = get_time_from_msg(*str);
    uint64_t cur_time = iclock64();
    std::cout << cur_time - send_time << ' ';
    std::cout.flush();
}

int main(int argc, char* argv[])
{
    try
    {
        // Check command line arguments.
        if (argc != 4 && argc != 5)
        {
            std::cerr << "Usage: battle_client_udt <local_port> <serv_address> <serv_port> <test_str_size>\n";
            std::cerr << "  For IPv4, try:\n";
            std::cerr << "    client_udt 12345 120.123,2.123 80 1460\n";
            return 1;
        }
    }
    catch (std::exception& e)
    {
        std::cerr << "exception: " << e.what() << "\n";
    }

	//ignore SIGPIPE
	sigset_t ps;
	sigemptyset(&ps);
	sigaddset(&ps, SIGPIPE);
	pthread_sigmask(SIG_BLOCK, &ps, NULL);

    size_t test_str_size = 1460;
    if (argc == 5)
        test_str_size = std::atoi(argv[4]);

    auto client = std::make_shared<UDTClient>(std::atoi(argv[1]), argv[2], std::atoi(argv[3]), recv_func);
    int start_ret = client->Start();
    if (start_ret == 0)
        return 0;

    while (true)
    {
        msg_ptr_t msg = make_test_str(test_str_size);
        client->SendMsg(msg);

        std::this_thread::sleep_for(std::chrono::milliseconds(20));
        size_t queue_size = client->SendMsgQueueSize();
        while (queue_size > 5)
        {
            std::cerr << "B";
            std::cerr.flush();
            std::this_thread::sleep_for(std::chrono::milliseconds(20));
            queue_size = client->SendMsgQueueSize();
        }
    }

    return 1;
}
