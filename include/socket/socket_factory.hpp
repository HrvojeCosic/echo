
#include "listener/server.hpp"

namespace echoserverclient {

class SocketFactory {
  public:
    static AbstractSocket createClientSocket(int argc, char *argv[]);
};
} // namespace echoserverclient