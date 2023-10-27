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
    void initInetSocketOptions(int socket);
    void acceptNewInetConnection();
    void acceptNewUnixConnection();
    void handleClientData(int clientIdx);
    void closeClientConnection(int clientIdx);

    std::vector<int> socketPool;
    std::vector<struct pollfd> pollFds;
    static constexpr int maxListeners = 5;
    static constexpr int maxClients = 150;
    static constexpr int bufferSize = 1024;

    uint16_t port;
    int inetListenSocket;

    int unixListenSocket;
    const std::string unixSocketPath;
};