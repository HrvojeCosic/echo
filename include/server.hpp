#pragma once

#include <poll.h>
#include <stop_token>
#include <unordered_map>

#include "./cli_command.hpp"
#include "./response_schema.hpp"
#include "./socket.hpp"

namespace echoserver {

const int maxClients = 50; // denotes max number of clients per listener

using AbstractResponseSchema = std::unique_ptr<IResponseSchema>;

//-----------------------------------------------------------------------------------------------------------------------------
class Server {
  public:
    Server();
    ~Server();
    Server(const Server &) = delete;
    void operator=(const Server &) = delete;
    Server(Server &&) = default;
    Server &operator=(Server &&) = delete;

    static void signalHandler(int signum);

    /* Sets up the socket listeners and starts the polling process with appropriate stopping handlers  */
    void start();

    /* Sets up the "listener" and adds it to the tracked socket state */
    void addListener(echoserverclient::AbstractSocket listener);

    inline void setResponseSchema(AbstractResponseSchema schema) { responseSchema = std::move(schema); };

    inline const AbstractResponseSchema &getResponseSchema() const { return responseSchema; }

    inline const std::vector<echoserverclient::AbstractSocket> &getListenerPool() const { return listenerPool; }

    inline const std::vector<int> &getClientPool() const { return clientPool; }

  private:
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
    int pollFileDescriptors();

    /* Receives data from socket of pollFdIdx and returns number of bytes read or throws an error if necessary, closing
     * the connection to that socket */
    int receiveFromClient(int pollFdIdx, char *buffer);

    /* Closes the socket of a client of id "clientIdx" and removes it from tracked client socket state  */
    void closeClientConnection(int clientIdx);

    /* Converts the index from pollFds to a corresponding client pool iterator. */
    inline std::vector<int>::iterator pollFdIdxToClientPoolIterator(int pollFdIdx) {
        return clientPool.begin() + pollFdIdx - listenerPool.size();
    }

    /* Gets the next iterator with the position for the next listener socket insertion into pollFds */
    inline std::vector<struct pollfd>::iterator getNextListenerPollFdsIterator() {
        return pollFds.begin() + listenerPool.size();
    }

    //-----------------------------------------------------------------------------------------------------------------------------
    /* Response schema dictates the way server is going to respond in */
    AbstractResponseSchema responseSchema;

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