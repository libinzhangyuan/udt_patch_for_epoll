Bench mark of the client_epoll with server_epoll.
This bench mark is for realtime pvp game.  For example, the multiplayer first person shooting game.
The requirement of realtime pvp game is packet is small and frequently. It wants a minimal delay. And the worst delay is not so worst.

The tests run server on a server on internet. The Bandwidth of internet is 5M.
And run client_epoll or client on home computer var wifi. The Bandwidth of ADSL is 10M.
Both Bandwidth is much bigger than we need (10 times more). And I use iptraf to verify it.

The client send 500 bytes data as one packet in every 50 milliseconds. Or 50 bytes for another test.
The server sendback the data after receiving the packet.

server is a udt epoll server.
./server/server 0.0.0.0 23456 >/dev/null

client_epoll is a client using udt epoll.
./client_epoll/client_epoll 20003 120.26.200.xxx 23456 500 2>>./error.log

client is a client using udt unblock socket by checking read/write frequently.
./client/client 20003 120.26.200.xxx 23456 500 2>>./error.log


The conclusion of this bench mark test:
The udt epoll is not good at realtime pvp game.
The latency is perfect at normal.
But the udt will fall into a badly situation if there is a small lag.


1. [Bench mark log for 500 bytes in every 50 milliseconds](https://github.com/libinzhangyuan/asio_kcp/blob/master/bench_mark/udt_500.log).
2. [Bench mark log for 50 bytes in every 50 milliseconds](https://github.com/libinzhangyuan/asio_kcp/blob/master/bench_mark/udt_50.log).
3. [Bench mark log for 500 bytes in every 50 milliseconds using another framework kcp](https://github.com/libinzhangyuan/asio_kcp/blob/master/bench_mark/kcp_500.log).  https://github.com/libinzhangyuan/asio_kcp
I run 2 and 3 at the same time. So you can verify the network.

The test log of client is some like client_epoll. I ignore the log from client.
