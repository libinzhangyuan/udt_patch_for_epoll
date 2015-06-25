#include <iostream>
#include <cstring>

#include <signal.h>
#include <cstdlib>

#include "udt_client.h"

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
        

    UDTClient client(std::atoi(argv[1]), argv[2], std::atoi(argv[3]), test_str_size);
    return 1;
}
