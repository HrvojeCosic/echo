#include <iostream>
#include <memory>
#include <stdexcept>

#include "../include/response_schema_factory.hpp"

namespace echoserver {

AbstractResponseSchema ResponseSchemaFactory::createSchema(const std::string &input, const uint schemaTypeOffset) {
    size_t spacePosition = input.find(' ', schemaTypeOffset);
    std::string schemaType = input.substr(schemaTypeOffset, spacePosition - schemaTypeOffset);

    if (schemaType == "EQUIVALENT") {
        return std::make_unique<EquivalentResponseSchema>();
    } else if (schemaType == "REVERSE") {
        return std::make_unique<ReverseResponseSchema>();
    } else if (schemaType == "CENSORED") {
        std::string keyword = "CHAR=";
        size_t keywordPosition = input.find(keyword);
        char censoredChar;
        if (keywordPosition != std::string::npos) {
            censoredChar = input[keywordPosition + keyword.length()];
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