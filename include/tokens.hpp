#include <memory>
#include <string>
#include <vector>

namespace echoserverclient {

class Tokens {
  public:
    virtual ~Tokens(){};
    Tokens(const std::vector<std::string> &tokens) : tokens(tokens){};
    Tokens(std::string input, char delimiter);

    virtual std::string getOption() const = 0;

    virtual std::string getChoice() const = 0;

    virtual std::string getChoiceArgument() const = 0;

    std::string detokenize(char delimiter);

  protected:
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