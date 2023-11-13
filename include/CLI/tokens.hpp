#pragma once

#include <memory>
#include <vector>

namespace echoserverclient {

class Tokens {
  public:
    virtual ~Tokens(){};
    Tokens(const std::vector<std::string> &tokens) : tokens(tokens){};
    Tokens(std::string input, char delimiter);

    /* Returns the token denoting the command's option (e.g --set-response-schema) if found in "tokens", otherwise
     * returns an empty string  */
    virtual std::string getOption() const = 0;

    /* Returns the token denoting the command's choice for the option (e.g CENSORED) if found in "tokens", otherwise
     * returns an empty string  */
    virtual std::string getChoice() const = 0;

    /* Returns the token denoting the command's argument for the option's choice (e.g CHAR=s) if found in "tokens",
     * otherwise returns an empty string  */
    virtual std::string getChoiceArgument() const = 0;

    /* Returns "tokens" concatenated into a string, separated by the "delimiter" */
    std::string detokenize(char delimiter);

  protected:
    /* Returns the token at "index" if such index exists, or an empty string otherwise */
    inline std::string getTokenAtIndex(size_t index) const { return tokens.size() <= index ? "" : tokens[index]; };

    std::vector<std::string> tokens;
};

class RuntimeTokens : public Tokens {
  public:
    using Tokens::Tokens;

    inline std::string getOption() const override { return getTokenAtIndex(0); };

    inline std::string getChoice() const override { return getTokenAtIndex(1); };

    inline std::string getChoiceArgument() const override { return getTokenAtIndex(2); };
};

class StartupTokens : public Tokens {
  public:
    using Tokens::Tokens;

    inline std::string getOption() const override { return getTokenAtIndex(1); };

    inline std::string getChoice() const override { return getTokenAtIndex(2); };

    inline std::string getExecutable() const { return getTokenAtIndex(0); };

    inline std::string getChoiceArgument() const override { return getTokenAtIndex(3); };
};

using AbstractTokens = std::unique_ptr<Tokens>;
} // namespace echoserverclient