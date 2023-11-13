#pragma once

#include <poll.h>
#include <stop_token>
#include <unordered_map>

#include "./cli_command.hpp"
#include "./listener.hpp"
#include "./response_schema.hpp"
#include "./socket.hpp"

namespace echoserver {

using AbstractResponseSchema = std::unique_ptr<IResponseSchema>;

//-----------------------------------------------------------------------------------------------------------------------------
class Server : public Listener {
  public:
    Server(std::string pipeName);
    ~Server();
    Server(const Server &) = delete;
    void operator=(const Server &) = delete;
    Server(Server &&) = default;
    Server &operator=(Server &&) = delete;

    static void signalHandler(int signum);

    /* Sets up the socket listeners and starts the polling process with appropriate stopping handlers  */
    void start();

    inline void setResponseSchema(AbstractResponseSchema schema) { responseSchema = std::move(schema); };

    inline const AbstractResponseSchema &getResponseSchema() const { return responseSchema; }

    inline const std::vector<int> &getClientPool() const { return clientPool; }

  private:
    //-----------------------------------------------------------------------------------------------------------------------------
    void handleClientData(int clientIdx);

    /* polls the client sockets, returning the number of ready sockets or throws an error if necessary  */
    int pollClientSockets();

    /* accepts incoming client connections from the pipe if there are any pending */
    void acceptIncomingClientConnections();

    /* Goes over all listeners and  */
    void handleIncomingData();

    /* Receives data from socket of pollFdIdx and returns number of bytes read or throws an error if necessary, closing
     * the connection to that socket */
    int receiveFromClient(int pollFdIdx, char *buffer);

    /* Closes the socket of a client of id "clientIdx" and removes it from tracked client socket state  */
    void closeClientConnection(int clientIdx);

    /* Takes client socket "fd" of another process, duplicates it for current process and returns it, or throws an error
     * if necessary */
    int duplicateClientFd(int fd);

    //-----------------------------------------------------------------------------------------------------------------------------
    /* name of the pipe from which the server receives new client socket file descriptors from the dispatcher */
    std::string pipeName;
    int pipeFd;

    /* Response schema dictates the way server is going to respond in */
    AbstractResponseSchema responseSchema;

    /* clientPool contains client socket file descriptors of clients connected to this server */
    std::vector<int> clientPool;

    /* clientFds contains the pollfd list of currently active clients */
    std::vector<struct pollfd> clientFds;
    //-----------------------------------------------------------------------------------------------------------------------------
};
} // namespace echoserver