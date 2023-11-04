#include "../include/client.hpp"
#include "../include/socket_factory.hpp"
#include <iostream>
#include <stdexcept>

int main(int argc, char *argv[]) {
    try {
        auto clientSocket = echoserverclient::SocketFactory::createClientSocket(argc, argv);
        std::vector<std::string> allArgs(argv, argv + argc);

        if (clientSocket == nullptr) {
            echoclient::Client client(nullptr);
            client.executeCommand(std::make_unique<echoserverclient::StartupTokens>("./echo_client --help", ' '));
            return 1;
        }

        echoclient::Client client(std::move(clientSocket));
        client.start();
    } catch (const std::runtime_error &e) {
        std::cerr << e.what() << std::endl;
    }

    return 0;
}