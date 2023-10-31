#include <csignal>
#include <iostream>
#include <iterator>
#include <sstream>
#include <stdexcept>
#include <sys/socket.h>

#include "../include/cli_command.hpp"
#include "../include/client.hpp"
#include "../include/response_schema_factory.hpp"
#include "../include/server.hpp"

namespace echoserverclient {

CLI_COMMAND_TYPE void CliCommand<APP_T>::tokenizeCliInput(std::string input, char delimiter,
                                                          std::vector<std::string> &tokens) {
    std::istringstream iss(input);
    std::string token;

    while (std::getline(iss, token, delimiter)) {
        tokens.emplace_back(token);
    }
}

CLI_COMMAND_TYPE std::string CliCommand<APP_T>::detokenizeCliInput(char delimiter,
                                                                   const std::vector<std::string> &tokens) {
    std::string result;
    for (size_t i = 0; i < tokens.size(); i++) {
        result += tokens[i];
        if (i < tokens.size() - 1) {
            result += delimiter;
        }
    }
    return result;
}

// TODO: maybe support futher help parameters (e.g --help change-response-schema)?
CLI_COMMAND_TYPE void HelpCliCommand<APP_T>::execute([[maybe_unused]] const std::vector<std::string> &tokens) const {
    if (std::same_as<APP_T, echoserver::Server>) {
        std::cout << "help server: " << std::endl;
    } else if (std::same_as<APP_T, echoclient::Client>) {
        std::cout << "help client: " << std::endl;
    }
}

void ChangeResponseSchemaCliCommand::execute(const std::vector<std::string> &tokens) const {
    auto responseSchema = echoserver::ResponseSchemaFactory::createSchema(tokens);
    if (responseSchema != nullptr) {
        app.setResponseSchema(std::move(responseSchema));
        std::cout << "Response schema has been changed" << std::endl;
    } else {
        std::cout << "Unknown response schema type or incorrect schema configuration" << std::endl;
    }
}

void SendToServerCliCommand::execute(const std::vector<std::string> &tokens) const {
    char buffer[app.getBufferSize()];
    int clientFd = app.getClientSocket()->getsocketFd();
    std::string userInput = detokenizeCliInput(' ', tokens);

    send(clientFd, userInput.c_str(), userInput.length(), 0);
    int bytesRead = recv(clientFd, buffer, sizeof(buffer), 0);

    if (bytesRead <= 0) {
        std::cerr << "Connection to the server terminated." << std::endl;
        app.getClientSocket()->destroy();
        echoclient::Client::signalHandler(SIGTERM);
    }

    std::string response(buffer, bytesRead);
    std::cout << "Server Response: " << response << std::endl;
}

template class CliCommand<echoserver::Server>;
template class CliCommand<echoclient::Client>;
template class HelpCliCommand<echoserver::Server>;
template class HelpCliCommand<echoclient::Client>;

} // namespace echoserverclient