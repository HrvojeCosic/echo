#pragma once

#include "./socket.hpp"
#include <sys/poll.h>
#include <vector>

namespace echoserver {

class Listener {
  public:
    virtual ~Listener();

    /* Sets up the "listener" and adds it to the tracked socket state */
    void addListenerSocket(echoserverclient::AbstractSocket listener);

    inline const std::vector<echoserverclient::AbstractSocket> &getListenerPool() const { return listenerPool; }

  protected:
    //-----------------------------------------------------------------------------------------------------------------------------
    /* Prepares listener sockets inside the listener pool */
    void prepareListenerSockets();

    /* Polls all file descriptors from pollFds, returning the number of file descriptors with events or throwing an
     * error if necessary */
    int pollListenerSockets();

    /* Finds a suitable server for an incoming client connection and forwards it to the appropriate process */
    void forwardIncomingConnection();

    //-----------------------------------------------------------------------------------------------------------------------------

    /* Sockets listening for requests */
    std::vector<echoserverclient::AbstractSocket> listenerPool;

    /* listener fds to poll for events */
    std::vector<struct pollfd> listenerFds;
};
} // namespace echoserver