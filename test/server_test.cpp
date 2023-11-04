#include <gtest/gtest.h>

#include "../include/server.hpp"
#include "../include/socket_factory.hpp"

using namespace echoclient;
using namespace echoserverclient;
using namespace echoserver;

class ServerTestFixture : public testing::Test {
  protected:
    std::unique_ptr<Server> server;

    void SetUp() override {
        const char *argv[] = {"./echo_server", "--set-response-schema", "REVERSE"};
        int argc = sizeof(argv) / sizeof(argv[0]);
        server = SocketFactory::createServer(argc, const_cast<char **>(argv));
    }
};

TEST_F(ServerTestFixture, ServerResponseSchemaTest) {
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
    // Default listeners (one unix domain listener, one internet domain listener)
    const UnixSocket *unixSocketDefault = dynamic_cast<const UnixSocket *>(server->getListenerPool()[0].get());
    const InetSocket *inetSocketDefault = dynamic_cast<const InetSocket *>(server->getListenerPool()[1].get());
    EXPECT_NE(unixSocketDefault, nullptr);
    EXPECT_NE(inetSocketDefault, nullptr);

    server->addListener(std::make_unique<InetSocket>(4000));
    const InetSocket *inetSocketAdded = dynamic_cast<const InetSocket *>(server->getListenerPool()[2].get());
    EXPECT_NE(inetSocketAdded, nullptr);
    EXPECT_EQ(server->getListenerPool().size(), 3);
}

TEST_F(ServerTestFixture, ServerCommandsTest) {
    std::cout << "---COMMAND TO CHANGE SCHEMA---" << std::endl;
    server->executeCommand(std::make_unique<StartupTokens>("./echo_server --set-response-schema CENSORED CHAR=s", ' '));
    auto *commandedSchema = dynamic_cast<CensoredResponseSchema *>(server->getResponseSchema().get());
    EXPECT_NE(commandedSchema, nullptr);

    std::cout << "---COMMAND TO PRINT USAGE HELP---" << std::endl;
    bool done = server->executeCommand(std::make_unique<RuntimeTokens>("--help", ' '));
    EXPECT_TRUE(done);
}

int main(int argc, char **argv) {
    testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}