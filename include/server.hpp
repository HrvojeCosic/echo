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
    Server() = default;
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

    inline const std::vector<echoserverclient::AbstractSocket> &getListenerPool() const { return listenerPool; }

    inline const std::vector<int> &getClientPool() const { return clientPool; }

  private:
    //-----------------------------------------------------------------------------------------------------------------------------
    void handleClientData(int clientIdx);

    int pollClientSockets();

    /* Goes over all listeners and accepts any incoming connection requests */
    void acceptIncomingConnections();

    /* Goes over all listeners and  */
    void handleIncomingData();

    /* Receives data from socket of pollFdIdx and returns number of bytes read or throws an error if necessary, closing
     * the connection to that socket */
    int receiveFromClient(int pollFdIdx, char *buffer);

    /* Closes the socket of a client of id "clientIdx" and removes it from tracked client socket state  */
    void closeClientConnection(int clientIdx);

    //-----------------------------------------------------------------------------------------------------------------------------
    /* Response schema dictates the way server is going to respond in */
    AbstractResponseSchema responseSchema;

    /* clientPool contains client socket file descriptors of clients connected to this server */
    std::vector<int> clientPool;

    /* clientFds contains the pollfd list of currently active clients */
    std::vector<struct pollfd> clientFds;
    //-----------------------------------------------------------------------------------------------------------------------------
};
} // namespace echoserver