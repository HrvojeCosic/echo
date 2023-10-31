#include "../include/server.hpp"
#include <iostream>
#include <memory>

int main() {
    auto &server = echoserver::Server::getInstance();

    const std::string unixSocketPath = "/tmp/unix_socket";
    const int port = 6000;
    auto unixSock = std::make_unique<echoserverclient::UnixSocket>(unixSocketPath);
    auto inetSock = std::make_unique<echoserverclient::InetSocket>(port);

    server.addListener(std::move(unixSock));
    server.addListener(std::move(inetSock));
    server.start();
}
