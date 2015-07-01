OLD_PWD="$( pwd )"

OS="LINUX"
#OS="OSX"

ARCH="IA32"

echo "" && echo "" && echo "[-------------------------------]" && echo "   udt4" && echo "[-------------------------------]" && \
    cd ./udt4/ && make -e os=$OS arch=$ARCH && \
echo "" && echo "" && echo "[-------------------------------]" && echo "   server_udt" && echo "[-------------------------------]" && \
    cd ../server_udt/ && make -e os=$OS arch=$ARCH && \
echo "" && echo "" && echo "[-------------------------------]" && echo "   server" && echo "[-------------------------------]" && \
    cd ../server/ && make -e os=$OS arch=$ARCH && \
echo "" && echo "" && echo "[-------------------------------]" && echo "   client_epoll" && echo "[-------------------------------]" && \
    cd ../client_epoll/ && make -e os=$OS arch=$ARCH && \
echo "" && echo "" && echo "[-------------------------------]" && echo "   client" && echo "[-------------------------------]" && \
    cd ../client/ && make -e os=$OS arch=$ARCH && \
echo "" && echo "" && echo "[-------------------------------]" && echo "   utest_server" && echo "[-------------------------------]" && \
    cd ../utest_server/ && make -e os=$OS arch=$ARCH && \
echo ""

# restore old path.
cd $OLD_PWD
