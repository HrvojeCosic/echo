#include <cstring>
#include <iostream>

#include "../include/socket_factory.hpp"

namespace echoserverclient {

AbstractSocket SocketFactory::createClientSocket(int argc, char *argv[]) {
    if (argc == 1 || strcmp(argv[1], "--help") == 0) {
        return nullptr;
    }

    const std::string serverAddress = argv[1];
    AbstractSocket socket;

    if (argc == 2) {
        socket = std::make_unique<UnixSocket>(serverAddress);
    } else if (argc == 3) {
        int serverPort = std::atoi(argv[2]);
        socket = std::make_unique<InetSocket>(serverPort);
    } else {
        return nullptr;
    }

    socket->connectToServer(serverAddress);
    return socket;
}

echoserver::Server &SocketFactory::createServer(int argc, char *argv[]) {
    auto &server = echoserver::Server::getInstance();

    std::vector<std::string> tokens;
    tokens.reserve(argc);
    if (argc >= 2) {
        for (int i = 1; i < argc; i++) {
            tokens.emplace_back(argv[i]);
        }

        if (server.executeCommand(tokens[0], tokens) == false) {
            server.executeCommand("--help", tokens);
        }
    }

    const std::string unixSocketPath = "/tmp/unix_socket";
    const int port = 6000;
    server.addListener(std::make_unique<UnixSocket>(unixSocketPath));
    server.addListener(std::make_unique<InetSocket>(port));

    return server;
}
} // namespace echoserverclient