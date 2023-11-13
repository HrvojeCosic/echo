#include "../include/dispatcher.hpp"
#include <iostream>

int main(int argc, char *argv[]) {
    try {
        auto dispatcher = echoserver::Dispatcher(3);

        std::vector<std::string> allArgs(argv, argv + argc);
        auto tokens = std::make_unique<echoserverclient::StartupTokens>(allArgs);
        auto optionTok = tokens->getOption();
        if (!optionTok.empty()) {
            if (dispatcher.executeCommand(std::move(tokens)) == false) {
                dispatcher.executeCommand(
                    std::make_unique<echoserverclient::StartupTokens>("./dispatcher --help", ' '));
            }
        }

        dispatcher.start();
    } catch (const std::runtime_error &e) {
        std::cerr << e.what() << std::endl;
    }

    return 0;
}
