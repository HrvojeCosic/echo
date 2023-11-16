#pragma once

#include <memory>

#define INVALID_SOCKET_FD -1

namespace echo {

const int bufferSize = 1024;

class ISocket {
  public:
    ISocket() : socketFd(INVALID_SOCKET_FD){};
    ISocket(const ISocket &other) = delete;
    ISocket &operator=(const ISocket &other) = delete;
    ISocket(ISocket &&other) = delete;
    ISocket &operator=(ISocket &&other) = delete;

    virtual ~ISocket() { close(socketFd); }

    /* Binds a socket for the server to listen on  */
    virtual void bind() = 0;

    /*Sets up the initial socket options*/
    virtual void initOptions(int socketFd) = 0;

    /*Accepts the incoming connection to the server and returns the incoming socket fd. */
    virtual int setupNewConnection() = 0;

    /* Connects to the server of "serverAddress" */
    virtual void connectToServer(const std::string &serverAddress) = 0;

    inline virtual int getsocketFd() const { return socketFd; };

  protected:
    int socketFd;
};

class UnixSocket : public ISocket {
  public:
    UnixSocket(const std::string path);

    void bind() override;
    void initOptions(int socketFd) override;
    int setupNewConnection() override;
    void connectToServer(const std::string &serverAddress) override;

    inline std::string getSocketPath() { return socketPath; }

  private:
    const std::string socketPath;
};

class InetSocket : public ISocket {
  public:
    InetSocket(int port);

    void bind() override;
    void initOptions(int socketFd) override;
    int setupNewConnection() override;
    void connectToServer(const std::string &serverAddress) override;

  private:
    int port;
};

using AbstractSocket = std::unique_ptr<ISocket>;
} // namespace echo
