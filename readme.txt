This is a patch for UDT 4.10 version.
This patch is for UDT epoll api. Because UDT epoll api has major bug.

And it is a example for UDT epoll api also.
UDT - udt.sourceforge.net

Build:
1. git clone git://git.code.sf.net/p/udt/git udt-git
2. copy udt-git/udt4 to ./udt_epoll_emample/ folder
3. modify the OS and ARCH in allmake.sh.
4. run # . allmake.sh


Run test:
server_udt/udt_server 127.0.0.1 12345
client_udt/udt_client 22222 127.0.0.1 12345 1400
or  sh launcher_clients.sh



Other notes:
1. The version of udt4 in the project is 4.10.
    -- The 4.11 version has a bug that can't be fixed:
         The UDT::epoll_wait always return with the situation of all socket need read.
         http://sourceforge.net/p/udt/discussion/393036/thread/56c105fe/
         http://sourceforge.net/p/udt/discussion/852996/thread/88d8bee5/
    -- You can test 4.11 version's bug by change branch to udt4.11_with_bug )
3 . udt4 in this project is coming from https://github.com/LabAdvComp/UDR. It fixed some bug by LabAdvComp. And I made some patch too.
