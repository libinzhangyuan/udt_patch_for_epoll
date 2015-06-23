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
        if (argc != 3)
        {
            std::cerr << "Usage: battle_client_udt <address> <port>\n";
            std::cerr << "  For IPv4, try:\n";
            std::cerr << "    client_udt 120.123,2.123 80\n";
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

    UDTClient client(12346, argv[1], std::atoi(argv[2]));
    return 1;
}
