#pragma once

#include <string>
#include <vector>

#include "./tokens.hpp"

// Forward declarations to avoid circular deps
namespace echo {
class Client;
}

namespace echo {
class Dispatcher;
}

#define CLI_COMMAND_TYPE                                                                                               \
    template <typename APP_T>                                                                                          \
    requires(std::same_as<APP_T, Dispatcher> || std::same_as<APP_T, Client>)

namespace echo {

CLI_COMMAND_TYPE class CliCommand {
  public:
    CliCommand(APP_T &app) : app(app){};
    virtual ~CliCommand(){};

    /* Executes the CLI command using tokenized parameters found in "tokens"  */
    virtual void execute(AbstractTokens tokens) const = 0;

  protected:
    APP_T &app;
};

CLI_COMMAND_TYPE class HelpCliCommand : public CliCommand<APP_T> {
  public:
    HelpCliCommand(APP_T &app) : CliCommand<APP_T>(app){};

    void execute(AbstractTokens tokens) const override;
};

class ChangeResponseSchemaCliCommand : public CliCommand<Dispatcher> {
  public:
    ChangeResponseSchemaCliCommand(Dispatcher &app) : CliCommand<Dispatcher>(app){};

    void execute(AbstractTokens tokens) const override;
};

class SendToServerCliCommand : public CliCommand<Client> {
  public:
    SendToServerCliCommand(Client &app) : CliCommand<Client>(app){};

    void execute(AbstractTokens tokens) const override;
};
} // namespace echo