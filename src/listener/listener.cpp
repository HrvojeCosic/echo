#include <iostream>
#include <sys/poll.h>
#include <sys/socket.h>

#include "listener/listener.hpp"

namespace echo {

namespace {
const int listenerQueueSize = 50;
}

Listener::~Listener() {
    for (auto &listener : getListenerPool()) {
        close(listener->getsocketFd());

        auto uds = dynamic_cast<UnixSocket *>(listener.get());
        if (uds) {
            unlink(uds->getSocketPath().c_str());
        }
    }
}

void Listener::addListenerSocket(AbstractSocket listener) {
    try {
        listener->bind();
        listener->initOptions(listener->getsocketFd());
        listenerFds.emplace_back(pollfd{listener->getsocketFd(), POLLIN, 0});
        listenerPool.emplace_back(std::move(listener));
    } catch (const std::runtime_error &e) {
        std::cerr << e.what() << std::endl;
    }
}

void Listener::prepareListenerSockets() {
    for (AbstractSocket &listener : listenerPool) {
        if (listen(listener->getsocketFd(), listenerQueueSize) == -1) {
            throw std::system_error(errno, std::generic_category(),
                                    "Failed to prepare for conection acceptance from dispatcher listener socket");
        }
    }
}

int Listener::pollListenerSockets() {
    int numReady = poll(listenerFds.data(), listenerFds.size(), -1);
    if (numReady == -1 && errno != EINTR) {
        throw std::system_error(errno, std::generic_category(), "Listener socket poll error");
    }
    return numReady;
}
} // namespace echo