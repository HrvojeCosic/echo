#include "../include/server.hpp"
#include <iostream>
#include <memory>

int main() {
    echoserver::Server server(std::make_unique<echoserver::ReverseResponseSchema>());

    const std::string unixSocketPath = "/tmp/unix_socket";
    const int port = 6000;
    auto unixSock = std::make_unique<echoserver::UnixSocket>(unixSocketPath);
    auto inetSock = std::make_unique<echoserver::InetSocket>(port);

    server.addListener(std::move(unixSock));
    server.addListener(std::move(inetSock));
    server.start();
}
