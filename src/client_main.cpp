#include <iostream>

#include "../include/client.hpp"
#include "../include/socket_factory.hpp"

int main(int argc, char *argv[]) {
    auto clientSocket = echoserverclient::SocketFactory::createClientSocket(argc, argv);

    if (clientSocket == nullptr) {
        std::string correctFormat = " (<Unix_domain_socket_address> | <Internet_domain_address> <port_number>)";
        std::cerr << "Usage: " << argv[0] << correctFormat << std::endl;
        return 1;
    }

    echoclient::Client client(std::move(clientSocket));
    client.start();

    return 0;
}