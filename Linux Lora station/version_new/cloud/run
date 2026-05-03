#!/bin/bash

echo "Cleaning old process..."
fuser -k 9001/tcp 2>/dev/null

echo "Compiling..."

g++ main.cpp -o ws -lboost_system -lpthread -lcurl
g++ client.cpp -o client -lboost_system -lpthread

if [ $? -ne 0 ]; then
    echo "Compile failed"
    exit 1
fi

echo "Starting server..."
./ws &

sleep 1

# echo "Starting client..."
# ./client

# sudo apt install -y build-essential python3 cmake libssl-dev zlib1g-dev git
# git clone https://github.com/uNetworking/uWebSockets.git
# cd uWebSockets
# git submodule update --init --recursive
#  sudo ln -s /usr/local/include/uWebSockets /usr/local/include/uwebsockets

# sudo mkdir -p /usr/local/include/uSockets
# sudo cp -r ~/firebase/uWebSockets/uSockets/src/* /usr/local/include/uSockets
# ls /usr/local/include/uSockets