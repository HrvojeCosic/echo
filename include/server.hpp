#pragma once

#include <poll.h>
#include <stop_token>
#include <unordered_map>

#include "./cli_command.hpp"
#include "./response_schema.hpp"
#include "./socket.hpp"

namespace echoserver {

using AbstractCliCommand = std::unique_ptr<echoserverclient::CliCommand<Server>>;
using AbstractResponseSchema = std::unique_ptr<IResponseSchema>;
using ResponseSchemaCliCommand = echoserverclient::ChangeResponseSchemaCliCommand;
using ServerHelpCliCommand = echoserverclient::HelpCliCommand<Server>;
using InputToCommandType = std::unordered_map<std::string, AbstractCliCommand>;

class Server {
  public:
    ~Server();
    Server(const Server &) = delete;
    void operator=(const Server &) = delete;
    Server(Server &&) = delete;
    Server &operator=(Server &&) = delete;

    static Server &getInstance();
    static void signalHandler(int signum);

    /* Sets up the socket listeners and starts the polling process with appropriate stopping handlers  */
    void start();

    /* Sets up the "listener" and adds it to the tracked socket state */
    void addListener(echoserverclient::AbstractSocket listener);

    /* Executes the command found in "tokens" if command has been setup inside "inputToCommand".
     * Returns true if command has been found, false otherwise
     */
    bool executeCommand(echoserverclient::AbstractTokens tokens);

    inline void setResponseSchema(AbstractResponseSchema schema) { responseSchema = std::move(schema); };

    inline const AbstractResponseSchema &getResponseSchema() const { return responseSchema; }

    inline const std::vector<echoserverclient::AbstractSocket> &getListenerPool() const { return listenerPool; }

    inline const std::vector<int> &getClientPool() const { return clientPool; }

  private:
    Server();

    //-----------------------------------------------------------------------------------------------------------------------------
    void handleClientData(int clientIdx);

    /* Prepares listener sockets inside the server's listener pool to accept connections */
    void prepareListeners();

    /* Goes over all listeners and accepts any incoming connection requests */
    void acceptIncomingConnections();

    /* Goes over all listeners and  */
    void handleIncomingData();

    /* Polls all file descriptors from pollFds, returning the number of file descriptors with events or throwing an
     * error if necessary */
    virtual int pollFileDescriptors();

    /* Receives data from socket of pollFdIdx and returns number of bytes read or throws an error if necessary, closing
     * the connection to that socket */
    virtual int receiveFromClient(int pollFdIdx, char *buffer);

    /* Closes the socket of a client of id "clientIdx" and removes it from tracked client socket state  */
    void closeClientConnection(int clientIdx);

    /* Listens for user input in a REPL, triggering requested commands until requested to stop */
    void cliInputHandler(std::stop_token token);

    /* Converts the index from pollFds to a corresponding client pool iterator. */
    inline std::vector<int>::iterator pollFdIdxToClientPoolIterator(int pollFdIdx) {
        return clientPool.begin() + pollFdIdx - listenerPool.size();
    }

    /* Gets the next iterator with the position for the next listener socket insertion into pollFds */
    inline std::vector<struct pollfd>::iterator getNextListenerPollFdsIterator() {
        return pollFds.begin() + listenerPool.size();
    }

    //-----------------------------------------------------------------------------------------------------------------------------
    const int maxListeners = 5;
    const int maxClients = 50; // denotes max number of clients per listener
    const int bufferSize = 1024;

    /* Response schema dictates the way server is going to respond in */
    AbstractResponseSchema responseSchema;

    /* inputToCommand maps user CLI inputs to their respective commands that hold command executors */
    InputToCommandType inputToCommand;

    /*
     *clientPool contains only client socket file descriptors
     *this means that their indices will correspond to elements at the same index in pollFd soffseted by number of
     *listeners
     */
    std::vector<int> clientPool;

    /*
     *listenerPool contains only the listener objects
     *this means that their indices will correspond to elements at the same index in pollFds
     */
    std::vector<echoserverclient::AbstractSocket> listenerPool;

    /*
     *pollFds contain all socket file descriptors, first number (equal to the number of listeners) of them being
     *listener sockets, and the rest being the client sockets.
     */
    std::vector<struct pollfd> pollFds;
    //-----------------------------------------------------------------------------------------------------------------------------
};
} // namespace echoserver