#include "../include/response_schema_factory.hpp"
#include "../include/tokens.hpp"
#include <gtest/gtest.h>

using namespace echoserverclient;
using namespace echoserver;

class ResponseSchemaFactoryTestFixture : public testing::Test {
  protected:
    AbstractResponseSchema createSchemaFromInput(std::string input) {
        AbstractTokens tokens = std::make_unique<RuntimeTokens>(input, ' ');
        AbstractResponseSchema schema = ResponseSchemaFactory::createSchema(std::move(tokens));
        return schema;
    }
};

TEST_F(ResponseSchemaFactoryTestFixture, CreateSchemaEquivalent) {
    auto schema = createSchemaFromInput("--set-response-schema EQUIVALENT");
    auto eqSchema = dynamic_cast<EquivalentResponseSchema *>(schema.get());
    EXPECT_NE(eqSchema, nullptr);
}

TEST_F(ResponseSchemaFactoryTestFixture, CreateSchemaReverse) {
    auto schema = createSchemaFromInput("--set-response-schema REVERSE");
    auto reverseSchema = dynamic_cast<ReverseResponseSchema *>(schema.get());
    EXPECT_NE(reverseSchema, nullptr);
}

TEST_F(ResponseSchemaFactoryTestFixture, CreateSchemaCensored) {
    auto schema = createSchemaFromInput("--set-response-schema CENSORED CHAR=s");
    auto censoredSchema = dynamic_cast<CensoredResponseSchema *>(schema.get());
    EXPECT_NE(censoredSchema, nullptr);
}

TEST_F(ResponseSchemaFactoryTestFixture, CreateSchemaPalindrome) {
    auto schema = createSchemaFromInput("--set-response-schema PALINDROME");
    auto palindromeSchema = dynamic_cast<PalindromeResponseSchema *>(schema.get());
    EXPECT_NE(palindromeSchema, nullptr);
}

TEST_F(ResponseSchemaFactoryTestFixture, CreateSchemaInvalid) {
    auto schema = createSchemaFromInput("--set-response-schema INVALID??");
    EXPECT_EQ(schema, nullptr);
}

int main(int argc, char **argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
