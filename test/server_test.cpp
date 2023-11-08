#include <gtest/gtest.h>

#include "../include/server.hpp"
#include "../include/socket_factory.hpp"

using namespace echoclient;
using namespace echoserverclient;
using namespace echoserver;

class ServerTestFixture : public testing::Test { };

TEST_F(ServerTestFixture, ServerResponseSchemaTest) {
    const char *argv[] = {"./echo_server", "--set-response-schema", "REVERSE"};
    int argc = sizeof(argv) / sizeof(argv[0]);
    Server server = SocketFactory::createServer(argc, const_cast<char **>(argv));

    std::string message = "hello";
    server.getResponseSchema()->generateResponse(message);
    EXPECT_EQ(message, "olleh");

    message = "hi";
    server.setResponseSchema(std::make_unique<EquivalentResponseSchema>());
    server.getResponseSchema()->generateResponse(message);
    EXPECT_EQ(message, "hi");

    message = "racecar";
    server.setResponseSchema(std::make_unique<PalindromeResponseSchema>());
    server.getResponseSchema()->generateResponse(message);
    EXPECT_EQ(message, "This message is a palindrome :)");

    message = "test";
    server.setResponseSchema(std::make_unique<CensoredResponseSchema>('t'));
    server.getResponseSchema()->generateResponse(message);
    EXPECT_EQ(message, "es");
}

TEST_F(ServerTestFixture, ServerListenersTest) {
    Server server;

    server.addListener(std::make_unique<InetSocket>(4000));
    ASSERT_EQ(server.getListenerPool().size(), 1);
    const InetSocket *inetSocketAdded = dynamic_cast<const InetSocket *>(server.getListenerPool()[0].get());
    EXPECT_NE(inetSocketAdded, nullptr);

    server.addListener(std::make_unique<UnixSocket>(""));
    ASSERT_EQ(server.getListenerPool().size(), 2);
    const UnixSocket *unixSocketAdded = dynamic_cast<const UnixSocket *>(server.getListenerPool()[1].get());
    EXPECT_NE(unixSocketAdded, nullptr);
}

TEST_F(ServerTestFixture, ServerCommandsTest) {
    Server server;
    std::cout << "---COMMAND TO CHANGE SCHEMA---" << std::endl;
    server.executeCommand(std::make_unique<StartupTokens>("./echo_server --set-response-schema CENSORED CHAR=s", ' '));
    auto *commandedSchema = dynamic_cast<CensoredResponseSchema *>(server.getResponseSchema().get());
    EXPECT_NE(commandedSchema, nullptr);

    std::cout << "---COMMAND TO PRINT USAGE HELP---" << std::endl;
    bool done = server.executeCommand(std::make_unique<RuntimeTokens>("--help", ' '));
    EXPECT_TRUE(done);
}

int main(int argc, char **argv) {
    testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}