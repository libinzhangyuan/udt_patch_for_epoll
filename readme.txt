This is a example for UDT epoll api.
UDT - udt.sourceforge.net

Build:
1. git clone git://git.code.sf.net/p/udt/git udt-git
2. copy udt-git/udt4 to ./udt_epoll_emample/ folder
3. modify the OS and ARCH in allmake.sh.
4. run # . allmake.sh


Run test:
server_udt/udt_server 127.0.0.1 12345
client_udt/udt_client 127.0.0.1 12345
