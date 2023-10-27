#include "../include/server.hpp"

int main() {
    const std::string unixSocketPath = "/tmp/unix_socket";
    Server server(5000, unixSocketPath);
    server.start();
}