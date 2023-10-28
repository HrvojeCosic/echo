#pragma once

#include <string>

namespace echoserver {

class ISocket {
  public:
    virtual ~ISocket() {}

    virtual int createAndBind() = 0;
    virtual void initOptions(int socketFd) = 0;
    virtual int setupNewConnection() = 0;
    virtual void destroy() = 0;

    inline virtual int getListenSocket() const { return listenSocket; };

  protected:
    int listenSocket;
};

class UnixSocket : public ISocket {
  public:
    UnixSocket(const std::string path) : socketPath(path){};

    int createAndBind() override;
    void initOptions(int socketFd) override;
    int setupNewConnection() override;
    void destroy() override;

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

  private:
    int port;
};
} // namespace echoserver
