
#include "../include/client.hpp"
#include <arpa/inet.h>
#include <csignal>
#include <fcntl.h>
#include <iostream>
#include <netinet/in.h>
#include <stop_token>
#include <sys/socket.h>
#include <sys/un.h>
#include <thread>

namespace echoclient {

volatile std::sig_atomic_t clientShutdownRequested = false;

void signalHandler(int signum) {
    if (signum == SIGINT || signum == SIGTERM) {
        clientShutdownRequested = true;
    }
}

void Client::closeClient() {
    if (clientSocket != -1) {
        close(clientSocket);
        clientSocket = -1;
    }
}

void Client::cliInputHandler(std::stop_token token) {
    char buffer[1024];

    while (!token.stop_requested()) {
        std::string userInput;
        std::cout << "Enter a message to send to the server: ";
        std::getline(std::cin, userInput);

        if (userInput.length() == 0)
            signalHandler(SIGTERM);

        send(clientSocket, userInput.c_str(), userInput.length(), 0);
        int bytesRead = recv(clientSocket, buffer, sizeof(buffer), 0);

        if (bytesRead <= 0) {
            std::cerr << "Connection to the server terminated." << std::endl;
            closeClient();
            signalHandler(SIGTERM);
            break;
        }

        std::string response(buffer, bytesRead);
        std::cout << "Server Response: " << response << std::endl;
    }
}

void Client::start(const std::string &serverAddress, int serverPort) {
    std::signal(SIGINT, signalHandler);
    std::signal(SIGTERM, signalHandler);

    bool isUnixDomain = serverPort == -1;
    if (isUnixDomain) {
        clientSocket = socket(AF_UNIX, SOCK_STREAM, 0);

        struct sockaddr_un serverAddr {};

        serverAddr.sun_family = AF_UNIX;
        strcpy(serverAddr.sun_path, serverAddress.c_str());
        serverAddr.sun_path[0] = 0;

        if (connect(clientSocket, (struct sockaddr *)&serverAddr, sizeof(serverAddr)) == -1) {
            std::cerr << "Failed to connect to the Unix Domain server." << std::endl;
            closeClient();
            return;
        }
    } else {
        clientSocket = socket(AF_INET, SOCK_STREAM, 0);

        struct sockaddr_in serverSockAddr {};

        serverSockAddr.sin_family = AF_INET;
        serverSockAddr.sin_port = htons(serverPort);
        inet_pton(AF_INET, serverAddress.c_str(), &serverSockAddr.sin_addr);

        if (connect(clientSocket, (struct sockaddr *)&serverSockAddr, sizeof(serverSockAddr)) == -1) {
            std::cerr << "Failed to connect to the Internet server." << std::endl;
            closeClient();
            return;
        }
    }

    std::jthread userInput([this](std::stop_token st) { this->cliInputHandler(st); });

    while (true) {
        if (clientShutdownRequested) {
            closeClient();
            std::cout << "\nClient closed. Press enter to continue." << std::endl;
            break;
        }
    }
}

} // namespace echoclient