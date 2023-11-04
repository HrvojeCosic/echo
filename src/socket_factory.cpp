#include <cstring>
#include <memory>

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

std::unique_ptr<echoserver::Server> SocketFactory::createServer(int argc, char *argv[]) {
    auto server = std::make_unique<echoserver::Server>();

    std::vector<std::string> allArgs(argv, argv + argc);
    auto tokens = std::make_unique<StartupTokens>(allArgs);
    auto optionTok = tokens->getOption();
    if (!optionTok.empty()) {
        if (server->executeCommand(std::move(tokens)) == false) {
            server->executeCommand(std::make_unique<StartupTokens>("./echo_server --help", ' '));
        }
    }

    const std::string unixSocketPath = "/tmp/unix_socket";
    const int port = 6000;
    server->addListener(std::make_unique<UnixSocket>(unixSocketPath));
    server->addListener(std::make_unique<InetSocket>(port));

    return server;
}
} // namespace echoserverclient