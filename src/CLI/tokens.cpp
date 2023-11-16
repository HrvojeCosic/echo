#include <sstream>

#include "CLI/tokens.hpp"

namespace echo {

Tokens::Tokens(std::string input, char delimiter) {
    std::istringstream iss(input);

    std::string currToken;
    while (std::getline(iss, currToken, delimiter)) {
        tokens.emplace_back(currToken);
    }
}

std::string Tokens::detokenize(char delimiter) {
    std::string result;

    for (size_t i = 0; i < tokens.size(); i++) {
        result += tokens[i];
        if (i < tokens.size() - 1) {
            result += delimiter;
        }
    }
    return result;
}
} // namespace echo