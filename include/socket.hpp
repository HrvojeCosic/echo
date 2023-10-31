#pragma once

#include <memory>
#include <string>

#define INVALID_SOCKET_FD -1

namespace echoserverclient {

class ISocket {
  public:
    ISocket() : socketFd(INVALID_SOCKET_FD){};
    virtual ~ISocket(){};

    virtual int createAndBind() = 0;
    virtual void initOptions(int socketFd) = 0;
    virtual int setupNewConnection() = 0;
    virtual void destroy() = 0;
    virtual void connectToServer(const std::string &serverAddress) = 0;

    inline virtual int getsocketFd() const { return socketFd; };

  protected:
    int socketFd;
};

class UnixSocket : public ISocket {
  public:
    UnixSocket(const std::string path) : socketPath(path){};

    int createAndBind() override;
    void initOptions(int socketFd) override;
    int setupNewConnection() override;
    void destroy() override;
    void connectToServer(const std::string &serverAddress) override;

  private:
    const std::string socketPath;
};

class InetSocket : public ISocket {
  public:
    InetSocket(int port) : port(port){};

    int createAndBind() override;
    void initOptions(int socketFd) override;
    int setupNewConnection() override;
    void destroy() override;
    void connectToServer(const std::string &serverAddress) override;

  private:
    int port;
};

using AbstractSocket = std::unique_ptr<echoserverclient::ISocket>;
} // namespace echoserverclient
