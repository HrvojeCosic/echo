#include <csignal>
#include <iostream>
#include <sys/socket.h>

#include "../include/client.hpp"
#include "../include/response_schema_factory.hpp"

namespace echoserverclient {

// TODO: maybe support futher help parameters (e.g --help change-response-schema)?
CLI_COMMAND_TYPE void HelpCliCommand<APP_T>::execute([[maybe_unused]] AbstractTokens tokens) const {
    if constexpr (std::same_as<APP_T, echoserver::Server>) {
        std::cout << "Usage: " << std::endl;
        std::cout << "On startup: "
                  << "./echo_server [OPTION]" << std::endl;
        std::cout << "During runtime: "
                  << "./echo_server [OPTION]" << std::endl;
        std::cout << "--set-response-schema "
                  << "EQUIVALENT/REVERSE/CENSORED CHAR=c/PALINDROME" << std::endl;
    } else if constexpr (std::same_as<APP_T, echoclient::Client>) {
        std::string correctFormat = " (<Unix_domain_socket_address> | <Internet_domain_address> <port_number>)";
        std::cout << "Usage: "
                  << "./echo_client " << correctFormat << std::endl;
    }
}

void ChangeResponseSchemaCliCommand::execute(AbstractTokens tokens) const {
    auto responseSchema = echoserver::ResponseSchemaFactory::createSchema(std::move(tokens));
    if (responseSchema != nullptr) {
        app.setResponseSchema(std::move(responseSchema));
        std::cout << "Response schema has been set" << std::endl;
    } else {
        std::cout << "Unknown response schema type or incorrect schema configuration" << std::endl;
    }
}

void SendToServerCliCommand::execute(AbstractTokens tokens) const {
    char buffer[echoserverclient::bufferSize];
    int clientFd = app.getClientSocket()->getsocketFd();
    std::string userInput = tokens->detokenize(' ');

    send(clientFd, userInput.c_str(), userInput.length(), 0);
    int bytesRead = recv(clientFd, buffer, sizeof(buffer), 0);

    if (bytesRead <= 0) {
        std::cout << "Connection to the server terminated." << std::endl;
        echoclient::Client::signalHandler(SIGTERM);
        return;
    }

    std::string response(buffer, bytesRead);
    std::cout << "Server Response: " << response << std::endl;
}

template class CliCommand<echoserver::Server>;
template class CliCommand<echoclient::Client>;
template class HelpCliCommand<echoserver::Server>;
template class HelpCliCommand<echoclient::Client>;

} // namespace echoserverclient