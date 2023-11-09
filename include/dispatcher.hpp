#pragma once

#include <vector>

#include "./cli_command.hpp"
#include "./listener.hpp"
#include "./server.hpp"

namespace echoserver {

const int startPort = 6000; // port of the dispatcher (server ports follow it)
const std::string startUnixPath = "/tmp/unix_socket";

using ResponseSchemaCliCommand = echoserverclient::ChangeResponseSchemaCliCommand;
using InputToCommandMap = std::unordered_map<std::string, std::unique_ptr<echoserverclient::CliCommand<Dispatcher>>>;
using DispatcherHelpCliCommand = echoserverclient::HelpCliCommand<Dispatcher>;

//-----------------------------------------------------------------------------------------------------------------------------
class Dispatcher : public Listener {
  public:
    Dispatcher(int initServerCount = 1);
    ~Dispatcher();
    Dispatcher(const Dispatcher &) = delete;
    void operator=(const Dispatcher &) = delete;
    Dispatcher(Dispatcher &&) = delete;
    Dispatcher &operator=(Dispatcher &&) = delete;

    static void signalHandler(int signum);

    /* Starts up the dispatcher REPL and server processes */
    void start();

    /* Executes the command found in "tokens" if command has been setup inside "inputToCommand".
     * Returns true if command has been found, false otherwise
     */
    bool executeCommand(echoserverclient::AbstractTokens tokens);

    void setResponseSchema(AbstractResponseSchema schema);

  private:
    //-----------------------------------------------------------------------------------------------------------------------------
    /* Listens for user input in a REPL, triggering requested commands until requested to stop */
    void cliInputHandler(std::stop_token token);

    /* Spins up another server process and populates serverPids with the newly created PID */
    void startServer();

    /* Finds a suitable server for an incoming client connection and forwards it to the appropriate process */
    void forwardIncomingConnections();

    /* inputToCommand maps user CLI inputs to their respective commands that hold command executors */
    InputToCommandMap inputToCommand;

    //-----------------------------------------------------------------------------------------------------------------------------
    int serverCount = 0;

    /* PIDs of processes running the currently active servers */
    std::vector<int> serverPids;
};
} // namespace echoserver