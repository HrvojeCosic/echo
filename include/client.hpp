#pragma once

#include <stop_token>
#include <string>

#include "./socket.hpp"

namespace echoclient {

class Client {
  public:
    Client(echoserverclient::AbstractSocket socket) : clientSocket(std::move(socket)){};

    void start();
    void cliInputHandler(std::stop_token token);

  private:
    const int bufferSize = 1024;

    echoserverclient::AbstractSocket clientSocket;
};
} // namespace echoclient