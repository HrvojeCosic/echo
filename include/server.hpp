#pragma once

#include <cstdint>
#include <memory>
#include <poll.h>
#include <string>
#include <vector>

#include "../include/socket.hpp"

namespace echoserver {

using AbstractSocket = std::unique_ptr<ISocket>;

class Server {
  public:
    Server();
    ~Server();

    void addListener(AbstractSocket listener);
    void start();

  private:
    //-----------------------------------------------------------------------------------------------------------------------------
    void handleClientData(int clientIdx);
    void closeClientConnection(int clientIdx);

    /* Converts the index from pollFds to a corresponding client pool iterator. */
    inline std::vector<int>::iterator pollFdIdxToClientPoolIterator(int pollFdIdx) {
        return clientPool.begin() + pollFdIdx - listenerPool.size() - 1;
    }

    /* Gets the next iterator with the position for the next listener socket insertion into pollFds */
    inline std::vector<struct pollfd>::iterator getNextListenerPollFdsIterator() {
        return pollFds.begin() + listenerPool.size();
    }

    //-----------------------------------------------------------------------------------------------------------------------------
    const int maxListeners = 5;
    const int maxClients = 50; // denotes max number of clients per listener
    const int bufferSize = 1024;

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
    std::vector<AbstractSocket> listenerPool;

    /*
     *pollFds contain all socket file descriptors, first number (equal to the number of listeners) of them being
     *listener sockets, and the rest being the client sockets.
     */
    std::vector<struct pollfd> pollFds;
    //-----------------------------------------------------------------------------------------------------------------------------
};
} // namespace echoserver