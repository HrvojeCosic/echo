#include <cstring>
#include <memory>

#include "../include/socket_factory.hpp"

namespace echoserverclient {

AbstractSocket SocketFactory::createClientSocket(int argc, char *argv[]) {
    std::vector<std::string> allArgs(argv, argv + argc);
    auto tokens = std::make_unique<StartupTokens>(allArgs);
    const std::string option = tokens->getOption();
    const std::string choice = tokens->getChoice();

    if (option.empty() || option == "--help") {
        return nullptr;
    }

    AbstractSocket socket;

    if (choice.empty()) {
        socket = std::make_unique<UnixSocket>(option);
    } else {
        int serverPort = std::atoi(choice.c_str());
        socket = std::make_unique<InetSocket>(serverPort);
    }

    socket->connectToServer(option);
    return socket;
}

echoserver::Server SocketFactory::createServer(int argc, char *argv[]) {
    echoserver::Server server;

    std::vector<std::string> allArgs(argv, argv + argc);
    auto tokens = std::make_unique<StartupTokens>(allArgs);
    auto optionTok = tokens->getOption();
    if (!optionTok.empty()) {
        if (server.executeCommand(std::move(tokens)) == false) {
            server.executeCommand(std::make_unique<StartupTokens>("./echo_server --help", ' '));
        }
    }

    const std::string unixSocketPath = "/tmp/unix_socket";
    const int port = 6000;
    server.addListener(std::make_unique<UnixSocket>(unixSocketPath));
    server.addListener(std::make_unique<InetSocket>(port));

    return server;
}
} // namespace echoserverclient