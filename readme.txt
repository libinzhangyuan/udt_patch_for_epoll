This is a patch for UDT 4.10 version.
This patch is for UDT epoll api. Because UDT epoll api has major bug.

And it is a example for UDT epoll api also.
UDT - udt.sourceforge.net

And it is a bench mark test. You can see the bench mark test in bench_mark.md.

### Build:
1. making gcc version >= 4.8
2. modify the OS and ARCH in allmake.sh.
3. $ . allmake.sh


### Run test:
server/server 127.0.0.1 12345
client_epoll/client_epoll 22222 127.0.0.1 12345 1400 2>>./state.log
or  sh launcher_clients.sh



### Compile for cppunit test
Install cppunit
*   download from http://nchc.dl.sourceforge.net/project/cppunit/cppunit/1.12.1/cppunit-1.12.1.tar.gz
        or you can use the pack in install_pack folder.
*   unpack it. cp cppunit-1.12.1 folder to the folder of duoduo project.
*   $ cd cppunit-1.12.1
*   $ ./configure
*   $ make



Other notes:
1. The version of udt4 in the project is 4.10.
    -- The 4.11 version has a bug that can't be fixed:
         The UDT::epoll_wait always return with the situation of all socket need read.
         http://sourceforge.net/p/udt/discussion/393036/thread/56c105fe/
         http://sourceforge.net/p/udt/discussion/852996/thread/88d8bee5/
    -- You can test 4.11 version's bug by change branch to udt4.11_with_bug   -  $ git checkout udt4.11_with_bug

2 . udt4 in this project is coming from https://github.com/LabAdvComp/UDR. It fixed some bug by LabAdvComp. And I made some patch too.

3. the meaning of state.log
  . - epoll_wait timeout return. means no socket prepare.
  N - send: socket is non-blocking (UDT_SNDSYN = false) but no buffer space is available.
  R - remove write event to socket
  A - add write event to socket
  B - epoll busy. sleep for stop add more msg to send buf.
