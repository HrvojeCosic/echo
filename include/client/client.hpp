#pragma once

#include <stop_token>
#include <unordered_map>

#include "CLI/cli_command.hpp"
#include "socket/socket.hpp"

namespace echo {

using AbstractCliCommand = std::unique_ptr<CliCommand<Client>>;
using ClientHelpCliCommand = HelpCliCommand<Client>;
using SendToServerCliCommand = SendToServerCliCommand;

class Client {
  public:
    Client(AbstractSocket socket);

    static void signalHandler(int signum);

    /* Enables the necessary handlers and stopping points for the client REPL */
    void start();

    /* Runs the client REPL and handles user input */
    void cliInputHandler(std::stop_token token);

    /* Executes the command found in "tokens" if command has been setup inside "inputToCommand".
     * Returns true if command has been found, false otherwise
     */
    bool executeCommand(AbstractTokens tokens);

    inline AbstractSocket &getClientSocket() { return clientSocket; };

  private:
    AbstractSocket clientSocket;

    /* inputToCommand maps user CLI inputs to their respective commands that hold command executors */
    std::unordered_map<std::string, AbstractCliCommand> inputToCommand;
};
} // namespace echo