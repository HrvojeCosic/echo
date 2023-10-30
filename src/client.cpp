#include <arpa/inet.h>
#include <csignal>
#include <fcntl.h>
#include <iostream>
#include <netinet/in.h>
#include <stop_token>
#include <sys/socket.h>
#include <sys/un.h>
#include <thread>

#include "../include/client.hpp"

namespace echoclient {

volatile std::sig_atomic_t clientShutdownRequested = false;

void signalHandler(int signum) {
    if (signum == SIGINT || signum == SIGTERM) {
        clientShutdownRequested = true;
    }
}

void Client::cliInputHandler(std::stop_token token) {
    char buffer[bufferSize];

    while (!token.stop_requested()) {
        std::string userInput;
        std::cout << "Enter a message to send to the server: ";
        std::getline(std::cin, userInput);

        if (userInput.length() == 0)
            signalHandler(SIGTERM);

        send(clientSocket->getsocketFd(), userInput.c_str(), userInput.length(), 0);
        int bytesRead = recv(clientSocket->getsocketFd(), buffer, sizeof(buffer), 0);

        if (bytesRead <= 0) {
            std::cerr << "Connection to the server terminated." << std::endl;
            clientSocket->destroy();
            signalHandler(SIGTERM);
            break;
        }

        std::string response(buffer, bytesRead);
        std::cout << "Server Response: " << response << std::endl;
    }
}

void Client::start() {
    std::signal(SIGINT, signalHandler);
    std::signal(SIGTERM, signalHandler);

    std::jthread userInput([this](std::stop_token st) { this->cliInputHandler(st); });

    while (true) {
        if (clientShutdownRequested) {
            clientSocket->destroy();
            userInput.request_stop();
            std::cout << "\nClient closed. Press enter to continue." << std::endl;
            break;
        }
    }
}

} // namespace echoclient