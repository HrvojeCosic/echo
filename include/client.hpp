#pragma once

#include <stop_token>
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

    /* Enables the necessary handlers and stopping points for the client REPL */
    void start();

    /* Runs the client REPL and handles user input */
    void cliInputHandler(std::stop_token token);

    /* Executes the command found in "tokens" if command has been setup inside "inputToCommand".
     * Returns true if command has been found, false otherwise
     */
    bool executeCommand(echoserverclient::AbstractTokens tokens);

    inline echoserverclient::AbstractSocket &getClientSocket() { return clientSocket; };

  private:
    echoserverclient::AbstractSocket clientSocket;

    /* inputToCommand maps user CLI inputs to their respective commands that hold command executors */
    std::unordered_map<std::string, AbstractCliCommand> inputToCommand;
};
} // namespace echoclient