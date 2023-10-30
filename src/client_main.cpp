#include <iostream>

#include "../include/client.hpp"

int main(int argc, char *argv[]) {
    if (argc != 2 && argc != 3) {
        std::string correctFormat = " (<Unix_domain_socket_address> | <Internet_domain_address> <port_number>)";
        std::cerr << "Usage: " << argv[0] << correctFormat << std::endl;
        return 1;
    }

    std::string serverAddress = argv[1];
    int serverPort = std::atoi(argv[2]);

    echoclient::Client client;
    client.start(serverAddress, serverPort);

    return 0;
}