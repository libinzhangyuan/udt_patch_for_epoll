OLD_PWD="$( pwd )"

    cd ./server_udt/ && make clean && \
    cd ../utest_server/ && make clean && \
    cd ../server/ && make clean && \
    cd ../client_epoll/ && make clean

# restore old path.
cd $OLD_PWD
