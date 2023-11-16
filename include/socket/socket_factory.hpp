
#include "listener/server.hpp"

namespace echo {

class SocketFactory {
  public:
    static AbstractSocket createClientSocket(int argc, char *argv[]);
};
} // namespace echo