#include "algorithm"

#include "../include/response_schema.hpp"
#include <iostream>

namespace echoserver {

void EquivalentResponseSchema::generateResponse([[maybe_unused]] std::string &message) { return; }

void ReverseResponseSchema::generateResponse(std::string &message) { std::reverse(message.begin(), message.end()); }

void CensoredResponseSchema::generateResponse(std::string &message) {
    message.erase(std::remove(message.begin(), message.end(), censoredChar), message.end());
}

void PalindromeResponseSchema::generateResponse(std::string &message) {
    int left_ptr = 0;
    int right_ptr = message.size() - 1;

    while (left_ptr <= right_ptr) {
        if (!isalnum(message[left_ptr])) {
            left_ptr++;
            continue;
        }
        if (!isalnum(message[right_ptr])) {
            right_ptr--;
            continue;
        }
        if (tolower(message[left_ptr]) != tolower(message[right_ptr])) {
            message = "This message is not a palindrome :(";
            return;
        } else {
            left_ptr++;
            right_ptr--;
        }
    }

    message = "This message is a palindrome :)";
}
} // namespace echoserver