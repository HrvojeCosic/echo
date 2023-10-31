#include <iostream>
#include <memory>
#include <stdexcept>

#include "../include/response_schema_factory.hpp"

namespace echoserver {

//TODO: make Token class to give names to tokens[1], tokens[2] etc.
AbstractResponseSchema ResponseSchemaFactory::createSchema(const std::vector<std::string> &tokens) {
    std::string schemaType = tokens[1];

    if (schemaType == "EQUIVALENT") {
        return std::make_unique<EquivalentResponseSchema>();
    } else if (schemaType == "REVERSE") {
        return std::make_unique<ReverseResponseSchema>();
    } else if (schemaType == "CENSORED") {
        std::string keyword = "CHAR=";
        size_t keywordPosition = tokens[2].find(keyword);
        char censoredChar;
        if (keywordPosition != std::string::npos) {
            censoredChar = tokens[2][keywordPosition + keyword.length()];
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