#include <cstring>
#include <memory>

#include "socket/socket_factory.hpp"

namespace echo {

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

} // namespace echo