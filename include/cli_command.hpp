#pragma once

#include <string>
#include <vector>

// Forward declarations to avoid circular deps
namespace echoclient {
class Client;
}

namespace echoserver {
class Server;
}

#define CLI_COMMAND_TYPE                                                                                               \
    template <typename APP_T>                                                                                          \
    requires(std::same_as<APP_T, echoserver::Server> || std::same_as<APP_T, echoclient::Client>)

namespace echoserverclient {

CLI_COMMAND_TYPE class CliCommand {
  public:
    CliCommand(APP_T &app) : app(app){};
    virtual ~CliCommand(){};

    static void tokenizeCliInput(std::string input, char delimiter, std::vector<std::string> &tokens);

    static std::string detokenizeCliInput(char delimiter, const std::vector<std::string> &tokens);

    virtual void execute(const std::vector<std::string> &tokens) const = 0;

  protected:
    APP_T &app;
};

CLI_COMMAND_TYPE class HelpCliCommand : public CliCommand<APP_T> {
  public:
    HelpCliCommand(APP_T &app) : CliCommand<APP_T>(app){};

    void execute(const std::vector<std::string> &tokens) const override;
};

class ChangeResponseSchemaCliCommand : public CliCommand<echoserver::Server> {
  public:
    ChangeResponseSchemaCliCommand(echoserver::Server &app) : CliCommand<echoserver::Server>(app){};

    void execute(const std::vector<std::string> &tokens) const override;
};

class SendToServerCliCommand : public CliCommand<echoclient::Client> {
  public:
    SendToServerCliCommand(echoclient::Client &app) : CliCommand<echoclient::Client>(app){};

    void execute(const std::vector<std::string> &tokens) const override;
};
} // namespace echoserverclient