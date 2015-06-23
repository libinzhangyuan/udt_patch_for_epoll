OLD_PWD="$( pwd )"

    cd ./server_udt/ && make clean && \
    cd ../client_udt/ && make clean

# restore old path.
cd $OLD_PWD
