#include "../include/response_schema_factory.hpp"

namespace echoserver {

AbstractResponseSchema ResponseSchemaFactory::createSchema(echoserverclient::AbstractTokens tokens) {
    std::string schemaType = tokens->getChoice();

    if (schemaType == "EQUIVALENT") {
        return std::make_unique<EquivalentResponseSchema>();
    } else if (schemaType == "REVERSE") {
        return std::make_unique<ReverseResponseSchema>();
    } else if (schemaType == "CENSORED") {
        std::string arg = tokens->getChoiceArgument();
        std::string keyword = "CHAR=";
        size_t keywordPosition = arg.find(keyword);
        char censoredChar;

        if (keywordPosition != std::string::npos) {
            censoredChar = arg[keywordPosition + keyword.length()];
        } else {
            return nullptr;
        }

        return std::make_unique<CensoredResponseSchema>(censoredChar);
    } else if (schemaType == "PALINDROME") {
        return std::make_unique<PalindromeResponseSchema>();
    } else {
        return nullptr;
    }
}

} // namespace echoserver