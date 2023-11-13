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

    inline const struct pollfd &getPipePollFd() const { return fdsToPoll[0]; }

    inline const int getClientFdStartIdx() const { return 1; }

  private:
    //-----------------------------------------------------------------------------------------------------------------------------
    void handleClientData(int clientIdx);

    /* polls the "fdsToPoll", returning the number of ready fds or throws an error if necessary  */
    int pollFileDescriptors();

    /* accepts incoming client connections from the pipe if there are any pending */
    void acceptIncomingClientConnections();

    /* Goes over all listeners and handles incoming data if there is any */
    void handleIncomingData();

    /* Receives data from socket of pollFdIdx and returns number of bytes read or throws an error if necessary, closing
     * the connection to that socket */
    int receiveFromClient(int pollFdIdx, char *buffer);

    /* Closes the socket of a client of id "pollFdIdx" and removes it from tracked client socket state  */
    void closeClientConnection(int pollFdIdx);

    /* Takes client socket "fd" of another process, duplicates it for current process and returns it, or throws an error
     * if necessary */
    int duplicateClientFd(int fd);

    //-----------------------------------------------------------------------------------------------------------------------------
    /* name of the pipe from which the server receives new client socket file descriptors from the dispatcher */
    std::string pipeName;

    /* Response schema dictates the way server is going to respond in */
    AbstractResponseSchema responseSchema;

    /* clientPool contains client socket file descriptors of clients connected to this server */
    std::vector<int> clientPool;

    /* pollFds contains the file descriptors to poll with the following format:
     * First element is the dispatcher->server pipe file descriptor.
     * The rest of the elements are file descriptors of currently active clients on this server
     */
    std::vector<struct pollfd> fdsToPoll;
    //-----------------------------------------------------------------------------------------------------------------------------
};
} // namespace echoserver