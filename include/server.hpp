#pragma once

#include <cstdint>
#include <memory>
#include <poll.h>
#include <stop_token>
#include <string>
#include <vector>

#include "../include/response_schema.hpp"
#include "../include/socket.hpp"

namespace echoserver {

using AbstractResponseSchema = std::unique_ptr<IResponseSchema>;

class Server {
  public:
    Server(AbstractResponseSchema responseSchema);

    void start();
    void closeServer();
    void addListener(echoserverclient::AbstractSocket listener);

    inline void setResponseSchema(AbstractResponseSchema schema) { responseSchema = std::move(schema); };

    inline const AbstractResponseSchema &getResponseSchema() const { return responseSchema; }

    inline const std::vector<echoserverclient::AbstractSocket> &getListenerPool() const { return listenerPool; }

    inline const std::vector<int> &getClientPool() const { return clientPool; }

  private:
    //-----------------------------------------------------------------------------------------------------------------------------
    void handleClientData(int clientIdx);
    void closeClientConnection(int clientIdx);
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