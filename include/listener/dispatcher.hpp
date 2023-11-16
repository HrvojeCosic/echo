#pragma once

#include <vector>

#include "CLI/cli_command.hpp"
#include "listener/listener.hpp"
#include "listener/server.hpp"

namespace echo {

const int startPort = 6000; // port of the dispatcher (server ports follow it)
const std::string startUnixPath = "/tmp/unix_socket";
const std::string startPipePath = "/tmp/fifo";

using InputToCommandMap = std::unordered_map<std::string, std::unique_ptr<CliCommand<Dispatcher>>>;
using DispatcherHelpCliCommand = HelpCliCommand<Dispatcher>;

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
    bool executeCommand(AbstractTokens tokens);

    inline const std::vector<std::pair<int, int>> &getServers() { return servers; }

    inline int getServerPidAtIdx(int idx) const { return servers[idx].first; }

    inline int getServerIdAtIdx(int idx) const { return servers[idx].second; }

    inline std::string serverIdToPipePath(int idx) const { return startPipePath + std::to_string(idx); }

    inline std::string serverIdToUnixPath(int idx) const { return startUnixPath + std::to_string(idx); }

    inline int serverIdToPort(int idx) const { return startPort + idx; }

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
    int latestServerId = 0;

    /* (TODO:to be deleted after introducing more balancing methods) variable tracking what is the index of a PID inside
     * serverPids that last received a client connection request  */
    int lastReceivingServerIdx = 0;

    /* Pairs <ProcessId, ServerId> representing processes running the currently active servers */
    // TODO: consider using pidfd instead of the pid for better safety
    std::vector<std::pair<int, int>> servers;
};
} // namespace echo