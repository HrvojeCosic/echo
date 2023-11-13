#include <fcntl.h>
#include <gtest/gtest.h>

#include "listener/server.hpp"
#include "socket/socket_factory.hpp"

using namespace echoclient;
using namespace echoserverclient;
using namespace echoserver;

class ServerTestFixture : public testing::Test {
    protected:
    void SetUp() override {
        std::string pipeName = "testpipe";
        mkfifo(pipeName.c_str(), O_RDWR);
        server = std::make_unique<Server>(pipeName);
    }
    std::unique_ptr<Server> server;
};

TEST_F(ServerTestFixture, ServerResponseSchemaTest) {
    server->setResponseSchema(std::make_unique<ReverseResponseSchema>());
    std::string message = "hello";
    server->getResponseSchema()->generateResponse(message);
    EXPECT_EQ(message, "olleh");

    message = "hi";
    server->setResponseSchema(std::make_unique<EquivalentResponseSchema>());
    server->getResponseSchema()->generateResponse(message);
    EXPECT_EQ(message, "hi");

    message = "racecar";
    server->setResponseSchema(std::make_unique<PalindromeResponseSchema>());
    server->getResponseSchema()->generateResponse(message);
    EXPECT_EQ(message, "This message is a palindrome :)");

    message = "test";
    server->setResponseSchema(std::make_unique<CensoredResponseSchema>('t'));
    server->getResponseSchema()->generateResponse(message);
    EXPECT_EQ(message, "es");
}

TEST_F(ServerTestFixture, ServerListenersTest) {
    server->addListenerSocket(std::make_unique<InetSocket>(4000));
    ASSERT_EQ(server->getListenerPool().size(), 1);
    const InetSocket *inetSocketAdded = dynamic_cast<const InetSocket *>(server->getListenerPool()[0].get());
    EXPECT_NE(inetSocketAdded, nullptr);

    server->addListenerSocket(std::make_unique<UnixSocket>(""));
    ASSERT_EQ(server->getListenerPool().size(), 2);
    const UnixSocket *unixSocketAdded = dynamic_cast<const UnixSocket *>(server->getListenerPool()[1].get());
    EXPECT_NE(unixSocketAdded, nullptr);
}

int main(int argc, char **argv) {
    testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}