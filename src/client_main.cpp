#include <iostream>

#include "../include/client.hpp"
#include "../include/socket_factory.hpp"

int main(int argc, char *argv[]) {
    auto clientSocket = echoserverclient::SocketFactory::createClientSocket(argc, argv);

    if (clientSocket == nullptr) {
        echoclient::Client client(nullptr);
        std::vector<std::string> tokens;
        client.executeCommand("--help", tokens);
        return 1;
    }

    echoclient::Client client(std::move(clientSocket));
    client.start();

    return 0;
}