#include <condition_variable>
#include <csignal>
#include <iostream>
#include <memory>
#include <system_error>
#include <thread>

#include "../include/dispatcher.hpp"

namespace echoserver {

namespace {
std::atomic_flag shutdownFlag = ATOMIC_FLAG_INIT;
std::condition_variable shutdownCV;
std::mutex shutdownMutex;
} // namespace

Dispatcher::Dispatcher(const int init_server_count) {
    servers.reserve(init_server_count);
    serverPids.reserve(init_server_count);

    int server_count = init_server_count;
    while (--server_count >= 0) {
        servers.emplace_back();
        int currPort = startPort + server_count;
        std::string currUnixPath = "/tmp/unix_socket" + std::to_string(server_count);
        servers.back().addListener(std::make_unique<echoserverclient::InetSocket>(currPort));
        servers.back().addListener(std::make_unique<echoserverclient::UnixSocket>(currUnixPath));
    }

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
        shutdownFlag.test_and_set();
        shutdownCV.notify_all();
    }
}

void Dispatcher::start() {
    std::signal(SIGINT, Dispatcher::signalHandler);
    std::signal(SIGTERM, Dispatcher::signalHandler);

    std::jthread userInput([this](std::stop_token st) { this->cliInputHandler(st); });

    for (auto &server : servers) {
        int pid = fork();
        if (pid < 0) {
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

    std::unique_lock<std::mutex> lock(shutdownMutex);
    shutdownCV.wait(lock, [] { return shutdownFlag.test(); });

    userInput.request_stop();
    std::cout << "\nDispatcher terminated." << std::endl;
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