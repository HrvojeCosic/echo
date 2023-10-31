#pragma once

#include <stop_token>
#include <string>
#include <unordered_map>

#include "./cli_command.hpp"
#include "./socket.hpp"

namespace echoclient {

using AbstractCliCommand = std::unique_ptr<echoserverclient::CliCommand<Client>>;
using ClientHelpCliCommand = echoserverclient::HelpCliCommand<Client>;
using SendToServerCliCommand = echoserverclient::SendToServerCliCommand;

class Client {
  public:
    Client(echoserverclient::AbstractSocket socket);

    static void signalHandler(int signum);

    void start();
    void cliInputHandler(std::stop_token token);

    inline echoserverclient::AbstractSocket &getClientSocket() { return clientSocket; };

    inline int getBufferSize() { return bufferSize; }

  private:
    int bufferSize = 1024;

    echoserverclient::AbstractSocket clientSocket;

    /* inputToCommand maps user CLI inputs to their respective commands that hold command executors */
    std::unordered_map<std::string, AbstractCliCommand> inputToCommand;
};
} // namespace echoclient