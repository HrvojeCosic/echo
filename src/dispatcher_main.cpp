#include "../include/dispatcher.hpp"
#include <iostream>

int main() {
    try {
        auto dispatcher = echoserver::Dispatcher(3);
        dispatcher.start();
    } catch (const std::runtime_error &e) {
        std::cerr << e.what() << std::endl;
    }

    return 0;
}
