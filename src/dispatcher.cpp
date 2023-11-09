#include <condition_variable>
#include <csignal>
#include <iostream>
#include <memory>
#include <sys/socket.h>
#include <system_error>
#include <thread>

#include "../include/dispatcher.hpp"

namespace echoserver {

namespace {
volatile std::sig_atomic_t shutdownRequested = false;
int listenerQueueSize = 50;
} // namespace

Dispatcher::Dispatcher(const int init_server_count) {
    servers.reserve(init_server_count);
    serverPids.reserve(init_server_count);

    // Setup dispatcher listeners
    addListener(std::make_unique<echoserverclient::InetSocket>(startPort));
    addListener(std::make_unique<echoserverclient::UnixSocket>(startUnixPath));

    int server_count = init_server_count + 1; // 1-indexed
    while (--server_count >= 1) {
        servers.emplace_back();

        // Setup server listeners
        int currPort = startPort + server_count;
        std::string currUnixPath = startUnixPath + std::to_string(server_count);
        servers.back().addListener(std::make_unique<echoserverclient::InetSocket>(currPort));
        servers.back().addListener(std::make_unique<echoserverclient::UnixSocket>(currUnixPath));
    }

    // Setup available dispatcher CLI commands
    inputToCommand["--set-response-schema"] = std::make_unique<ResponseSchemaCliCommand>(*this);
    inputToCommand["--help"] = std::make_unique<DispatcherHelpCliCommand>(*this);
}

Dispatcher::~Dispatcher() {
    for (int pid : serverPids) {
        kill(pid, SIGTERM);
    }
}

void Dispatcher::signalHandler(int signum) {
    if (signum == SIGINT || signum == SIGTERM) {
        shutdownRequested = true;
    }
}

void Dispatcher::start() {
    std::signal(SIGINT, Dispatcher::signalHandler);
    std::signal(SIGTERM, Dispatcher::signalHandler);

    prepareListeners();

    std::jthread userInput([this](std::stop_token st) { this->cliInputHandler(st); });

    for (auto &server : servers) {
        prepareServer(server);
    }

    while (!shutdownRequested) {
        if (pollFileDescriptors() != -1) {
            forwardIncomingConnection();
        }
    }

    userInput.request_stop();
    std::cout << "\nDispatcher terminated." << std::endl;
}

void Dispatcher::forwardIncomingConnection() {
    // TODO
}

void Dispatcher::prepareServer(Server &server) {
    int pid = fork();
    if (pid == -1) {
        throw std::system_error(errno, std::generic_category(), "Failed to start the new server process");
    } else if (pid == 0) {
        // Child process
        server.start();
        exit(0);
    } else {
        // Parent process
        serverPids.push_back(pid);
    }
}

void Dispatcher::addListener(echoserverclient::AbstractSocket listener) {
    try {
        listener->bind();
        listener->initOptions(listener->getsocketFd());
        pollFds.emplace_back(pollfd{listener->getsocketFd(), POLLIN, 0});
        listenerPool.emplace_back(std::move(listener));
    } catch (const std::runtime_error &e) {
        std::cerr << e.what() << std::endl;
    }
}

void Dispatcher::prepareListeners() {
    for (echoserverclient::AbstractSocket &listener : listenerPool) {
        if (listen(listener->getsocketFd(), listenerQueueSize) == -1) {
            throw std::system_error(errno, std::generic_category(),
                                    "Failed to prepare for conection acceptance from dispatcher listener socket");
        }
    }
}

int Dispatcher::pollFileDescriptors() {
    int numReady = poll(pollFds.data(), pollFds.size(), -1);
    if (numReady == -1 && errno != EINTR) {
        throw std::system_error(errno, std::generic_category(), "Poll error");
    }
    return numReady;
}

void Dispatcher::cliInputHandler(std::stop_token token) {
    while (!token.stop_requested()) {
        std::string userInput;
        std::getline(std::cin, userInput);

        if (userInput.length() == 0)
            continue;

        auto tokens = std::make_unique<echoserverclient::RuntimeTokens>(userInput, ' ');
        auto optionToken = tokens->getOption();
        if (inputToCommand.contains(optionToken)) {
            inputToCommand[optionToken]->execute(std::move(tokens));
        } else {
            inputToCommand["--help"]->execute(std::move(tokens));
        }
    }
}

bool Dispatcher::executeCommand(echoserverclient::AbstractTokens tokens) {
    auto command = tokens->getOption();
    bool exists = inputToCommand.find(command) != inputToCommand.end();

    if (exists) {
        inputToCommand[command]->execute(std::move(tokens));
    }

    return exists;
}

void Dispatcher::setResponseSchema(AbstractResponseSchema schema) {
    // TODO
}
} // namespace echoserver