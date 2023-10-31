#include <iostream>
#include <memory>
#include <unordered_map>

#include "../include/server.hpp"
#include "../include/socket_factory.hpp"

int main(int argc, char *argv[]) {
    auto &server = echoserverclient::SocketFactory::createServer(argc, argv);
    server.start();
    return 0;
}
