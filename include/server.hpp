#include <cstdint>
#include <poll.h>
#include <string>
#include <vector>

class Server {
  public:
    Server(int port, const std::string unixSocketPath);
    ~Server();

    void start();

  private:
    int createAndBindInetSocket();
    int createAndBindUnixSocket(const std::string &socketPath);
    void configureSockets();

    std::vector<int> clientSockets;
    std::vector<struct pollfd> pollFds;
    const int maxClients = 150;
    const int bufferSize = 1024;

    uint16_t port;
    int inetListenSocket;

    int unixListenSocket;
    const std::string unixSocketPath;
};