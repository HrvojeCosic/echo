#pragma once

#include <string>

namespace echo {

class IResponseSchema {
  public:
    virtual ~IResponseSchema(){};

    /* Modifies "message" based on the rules of the concrete schema */
    virtual void generateResponse(std::string &message) = 0;
};

class EquivalentResponseSchema : public IResponseSchema {
  public:
    void generateResponse(std::string &message) override;
};

class ReverseResponseSchema : public IResponseSchema {
  public:
    void generateResponse(std::string &message) override;
};

class CensoredResponseSchema : public IResponseSchema {
  public:
    CensoredResponseSchema(const char censored) : censoredChar(censored){};
    void generateResponse(std::string &message) override;

  private:
    const char censoredChar;
};

class PalindromeResponseSchema : public IResponseSchema {
  public:
    void generateResponse(std::string &message) override;
};
} // namespace echo