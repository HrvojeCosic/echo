#include <gtest/gtest.h>

#include "../include/dispatcher.hpp"

using namespace echoclient;
using namespace echoserverclient;
using namespace echoserver;

class DispatcherTestFixture : public testing::Test {};

//TODO: sort out the Dispatcher::setResponseSchema method first
//TEST_F(DispatcherTestFixture, DispatcherCommandsTest) {
//    Dispatcher dispatcher;
//    std::cout << "---COMMAND TO CHANGE SCHEMA---" << std::endl;
//    dispatcher.executeCommand(std::make_unique<StartupTokens>("./echo_server --set-response-schema CENSORED CHAR=s", ' '));
//    auto *commandedSchema = dynamic_cast<CensoredResponseSchema *>(server.getResponseSchema().get());
//    EXPECT_NE(commandedSchema, nullptr);
//
//    std::cout << "---COMMAND TO PRINT USAGE HELP---" << std::endl;
//    bool done = server.executeCommand(std::make_unique<RuntimeTokens>("--help", ' '));
//    EXPECT_TRUE(done);
//}

int main(int argc, char **argv) {
    testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}