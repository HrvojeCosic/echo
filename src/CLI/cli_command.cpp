#include <csignal>
#include <cstring>
#include <fcntl.h>
#include <iostream>
#include <sys/socket.h>

#include "client/client.hpp"
#include "listener/dispatcher.hpp"
#include "listener/response_schema_factory.hpp"

namespace echoserverclient {

// TODO: maybe support futher help parameters (e.g --help change-response-schema)?
CLI_COMMAND_TYPE void HelpCliCommand<APP_T>::execute([[maybe_unused]] AbstractTokens tokens) const {
    if constexpr (std::same_as<APP_T, echoserver::Dispatcher>) {
        std::cout << "Usage: " << std::endl;
        std::cout << "On startup: "
                  << "sudo ./dispatcher [OPTION]" << std::endl;
        std::cout << "During runtime: "
                  << "[OPTION]" << std::endl;
        std::cout << "--set-response-schema "
                  << "EQUIVALENT/REVERSE/CENSORED CHAR=c/PALINDROME" << std::endl;
    } else if constexpr (std::same_as<APP_T, echoclient::Client>) {
        std::string correctFormat = " (<Unix_domain_socket_address> | <Internet_domain_address> <port_number>)";
        std::cout << "Usage: "
                  << "./echo_client " << correctFormat << std::endl;
    }
}

void ChangeResponseSchemaCliCommand::execute(AbstractTokens tokens) const {
    auto servers = app.getServers();
    for (int i = 0; i < servers.size(); i++) {
        int pipeFd = open(app.serverIdToPipePath(app.getServerIdAtIdx(i)).c_str(), O_WRONLY);
        if (pipeFd == -1) {
            throw std::system_error(errno, std::generic_category(), "Failed to open the dispatcher->server pipe");
        }

        auto command = tokens->detokenize(' ');
        uint16_t commandLen = command.size();

        std::vector<std::byte> commandBuf;
        commandBuf.resize(sizeof(echoserver::commandPipeFlag) + sizeof(commandLen) + commandLen);
        commandBuf[0] = echoserver::commandPipeFlag;
        std::memcpy(&commandBuf[sizeof(echoserver::commandPipeFlag)], &commandLen, sizeof(commandLen));
        std::memcpy(&commandBuf[sizeof(echoserver::commandPipeFlag) + sizeof(commandLen)], command.c_str(), commandLen);

        ssize_t bytesWritten = write(pipeFd, commandBuf.data(), commandBuf.size());
        if (bytesWritten == -1) {
            close(pipeFd);
            throw std::system_error(errno, std::generic_category(), "Failed to write to the dispatcher->server pipe");
        }

        close(pipeFd);
    }
}

void SendToServerCliCommand::execute(AbstractTokens tokens) const {
    char buffer[echoserverclient::bufferSize];
    int clientFd = app.getClientSocket()->getsocketFd();
    std::string userInput = tokens->detokenize(' ');

    int bytesSent = send(clientFd, userInput.c_str(), userInput.length(), 0);
    if (bytesSent == -1) {
        std::cerr << "Couldn't send data to server. " << errno << std::endl;
    }

    int bytesRead = recv(clientFd, buffer, sizeof(buffer), 0);
    if (bytesRead <= 0) {
        std::cout << "Connection to the server terminated." << std::endl;
        echoclient::Client::signalHandler(SIGTERM);
        return;
    }

    std::string response(buffer, bytesRead);
    std::cout << "Server Response: " << response << std::endl;
}

template class CliCommand<echoserver::Dispatcher>;
template class CliCommand<echoclient::Client>;
template class HelpCliCommand<echoserver::Dispatcher>;
template class HelpCliCommand<echoclient::Client>;

} // namespace echoserverclient