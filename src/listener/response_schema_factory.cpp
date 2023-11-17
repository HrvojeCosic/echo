#include "listener/response_schema_factory.hpp"

namespace echo {

AbstractResponseSchema ResponseSchemaFactory::createSchema(const AbstractTokens &tokens) {
    switch (getSchemaType(tokens->getChoice())) {
        case SchemaType::EQUIVALENT:
            return std::make_unique<EquivalentResponseSchema>();
        case SchemaType::REVERSE:
            return std::make_unique<ReverseResponseSchema>();
        case SchemaType::CENSORED:
            return newCensoredSchema(tokens);
        case SchemaType::PALINDROME:
            return std::make_unique<PalindromeResponseSchema>();
        default:
            return nullptr;
    }
}

SchemaType ResponseSchemaFactory::getSchemaType(const std::string& schemaType) {
    static const std::unordered_map<std::string, SchemaType> choiceToType = {
        {"EQUIVALENT", SchemaType::EQUIVALENT},
        {"REVERSE", SchemaType::REVERSE},
        {"CENSORED", SchemaType::CENSORED},
        {"PALINDROME", SchemaType::PALINDROME}
    };

    auto it = choiceToType.find(schemaType);
    if (it != choiceToType.end()) {
        return it->second;
    } else {
        return SchemaType::UNKNOWN;
    }
}

std::unique_ptr<CensoredResponseSchema> ResponseSchemaFactory::newCensoredSchema(const AbstractTokens &tokens) {
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
}
} // namespace echo