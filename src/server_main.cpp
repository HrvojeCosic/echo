#include "../include/socket_factory.hpp"

int main(int argc, char *argv[]) {
    auto &server = echoserverclient::SocketFactory::createServer(argc, argv);
    server.start();
    return 0;
}
