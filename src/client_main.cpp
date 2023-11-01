#include <iostream>
#include <memory>

#include "../include/client.hpp"
#include "../include/socket_factory.hpp"

int main(int argc, char *argv[]) {
    auto clientSocket = echoserverclient::SocketFactory::createClientSocket(argc, argv);
    std::vector<std::string> allArgs(argv, argv + argc);

    if (clientSocket == nullptr) {
        echoclient::Client client(nullptr);
        client.executeCommand("--help", std::make_unique<echoserverclient::StartupTokens>(allArgs));
        return 1;
    }

    echoclient::Client client(std::move(clientSocket));
    client.start();

    return 0;
}