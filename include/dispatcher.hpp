#include "./server.hpp"
#include "cli_command.hpp"
#include <vector>

namespace echoserver {

const int startPort = 6000; // port of the first server

using ResponseSchemaCliCommand = echoserverclient::ChangeResponseSchemaCliCommand;
using InputToCommandMap = std::unordered_map<std::string, std::unique_ptr<echoserverclient::CliCommand<Dispatcher>>>;
using DispatcherHelpCliCommand = echoserverclient::HelpCliCommand<Dispatcher>;

//-----------------------------------------------------------------------------------------------------------------------------
class Dispatcher {
  public:
    Dispatcher(const int init_server_count = 1);
    ~Dispatcher();
    Dispatcher(const Dispatcher &) = delete;
    void operator=(const Dispatcher &) = delete;
    Dispatcher(Dispatcher &&) = delete;
    Dispatcher &operator=(Dispatcher &&) = delete;

    static void signalHandler(int signum);

    /* Starts up the dispatcher process */
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

    /* inputToCommand maps user CLI inputs to their respective commands that hold command executors */
    InputToCommandMap inputToCommand;

    //-----------------------------------------------------------------------------------------------------------------------------
    /* Currently active servers */
    std::vector<Server> servers;

    /* PIDs of processes running the servers */
    std::vector<int> serverPids;
};
} // namespace echoserver