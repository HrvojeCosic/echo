#include <condition_variable>
#include <csignal>
#include <iostream>
#include <thread>

#include "client/client.hpp"

namespace echo {

namespace {
std::atomic_flag clientShutdownFlag = ATOMIC_FLAG_INIT;
std::condition_variable clientShutdownCV;
std::mutex clientShutdownMutex;
} // namespace

Client::Client(AbstractSocket socket) : clientSocket(std::move(socket)) {
    inputToCommand["--help"] = std::make_unique<ClientHelpCliCommand>(*this);
    inputToCommand["send-to-server"] = std::make_unique<SendToServerCliCommand>(*this);
};

void Client::signalHandler(int signum) {
    if (signum == SIGINT || signum == SIGTERM) {
        clientShutdownFlag.test_and_set();
        clientShutdownCV.notify_all();
    }
}

bool Client::executeCommand(AbstractTokens tokens) {
    auto command = tokens->getOption();
    bool exists = inputToCommand.find(command) != inputToCommand.end();

    if (exists) {
        inputToCommand[command]->execute(std::move(tokens));
    }

    return exists;
}

void Client::cliInputHandler(std::stop_token token) {
    while (!token.stop_requested()) {
        std::string userInput;
        std::getline(std::cin, userInput);

        if (userInput.length() == 0) {
            Client::signalHandler(SIGTERM);
            break;
        }

        auto tokens = std::make_unique<RuntimeTokens>(userInput);
        auto optionToken = tokens->getOption();
        if (inputToCommand.contains(optionToken)) {
            inputToCommand[optionToken]->execute(std::move(tokens));
        } else {
            inputToCommand["send-to-server"]->execute(std::move(tokens));
        }
    }
}

void Client::start() {
    std::signal(SIGINT, signalHandler);
    std::signal(SIGTERM, signalHandler);

    std::jthread userInput([this](std::stop_token st) { this->cliInputHandler(st); });

    std::unique_lock<std::mutex> lock(clientShutdownMutex);
    clientShutdownCV.wait(lock, [] { return clientShutdownFlag.test(); });

    userInput.request_stop();
    std::cout << "\nClient closed." << std::endl;
}

} // namespace echo