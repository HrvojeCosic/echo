#pragma once

#include <stop_token>
#include <string>

namespace echoclient {

class Client {
  public:
    Client() : clientSocket(-1){};

    void start(const std::string &serverAddress, int serverPort = -1);
    void closeClient();
    void cliInputHandler(std::stop_token token);

  private:
    int clientSocket;
};
} // namespace echoclient