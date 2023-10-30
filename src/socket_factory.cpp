

#include "../include/socket_factory.hpp"

namespace echoserverclient {

AbstractSocket SocketFactory::createClientSocket(int argc, char *argv[]) {
    if (argc == 1)
        return nullptr;

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
} // namespace echoserverclient