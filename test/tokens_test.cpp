#include "CLI/tokens.hpp"
#include <gtest/gtest.h>

namespace echo {

TEST(Tokens, StartupTokensTokenizeAndDetokenize) {
    std::string inputStartup = "./executable --option choice argument";
    char delimiter = ' ';
    StartupTokens tokensStartup(inputStartup, delimiter);

    EXPECT_EQ(tokensStartup.getExecutable(), "./executable");
    EXPECT_EQ(tokensStartup.getOption(), "--option");
    EXPECT_EQ(tokensStartup.getChoice(), "choice");
    EXPECT_EQ(tokensStartup.getChoiceArgument(), "argument");

    EXPECT_EQ(tokensStartup.detokenize(delimiter), inputStartup);
}

TEST(Tokens, RuntimeTokensTokenizeAndDetokenize) {
    std::string inputRuntime = "--option choice argument";
    char delimiter = ' ';
    RuntimeTokens tokensRuntime(inputRuntime, delimiter);

    EXPECT_EQ(tokensRuntime.getOption(), "--option");
    EXPECT_EQ(tokensRuntime.getChoice(), "choice");
    EXPECT_EQ(tokensRuntime.getChoiceArgument(), "argument");

    EXPECT_EQ(tokensRuntime.detokenize(delimiter), inputRuntime);
}

int main(int argc, char **argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
} // namespace echo