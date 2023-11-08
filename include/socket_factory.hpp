
#include "./server.hpp"
#include "./socket.hpp"

namespace echoserverclient {

class SocketFactory {
  public:
    static AbstractSocket createClientSocket(int argc, char *argv[]);
    static echoserver::Server createServer(int argc, char *argv[]);
};
} // namespace echoserverclient