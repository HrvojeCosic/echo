#include <condition_variable>
#include <csignal>
#include <iostream>
#include <stop_token>
#include <thread>

#include "../include/client.hpp"

namespace echoclient {

std::atomic_flag clientShutdownFlag = ATOMIC_FLAG_INIT;
std::condition_variable clientShutdownCV;
std::mutex clientShutdownMutex;

Client::Client(echoserverclient::AbstractSocket socket) : clientSocket(std::move(socket)) {
    inputToCommand["--help"] = std::make_unique<ClientHelpCliCommand>(*this);
    inputToCommand["send-to-server"] = std::make_unique<SendToServerCliCommand>(*this);
};

void Client::signalHandler(int signum) {
    if (signum == SIGINT || signum == SIGTERM) {
        clientShutdownFlag.test_and_set();
        clientShutdownCV.notify_all();
    }
}

bool Client::executeCommand(std::string command, std::vector<std::string> &tokens) {
    bool exists = inputToCommand.find(command) != inputToCommand.end();

    if (exists) {
        inputToCommand[command]->execute(tokens);
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

        std::vector<std::string> tokens;
        echoserverclient::CliCommand<Client>::tokenizeCliInput(userInput, ' ', tokens);
        if (inputToCommand.contains(tokens[0])) {
            inputToCommand[tokens[0]]->execute(tokens);
        } else {
            inputToCommand["send-to-server"]->execute(tokens);
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

} // namespace echoclient