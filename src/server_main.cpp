#include "../include/socket_factory.hpp"
#include <iostream>

int main(int argc, char *argv[]) {
    try {
        auto server = echoserverclient::SocketFactory::createServer(argc, argv);
        server->start();
    } catch (const std::runtime_error &e) {
        std::cerr << e.what() << std::endl;
    }

    return 0;
}
