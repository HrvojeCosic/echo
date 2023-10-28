#pragma once

#include <string>

namespace echoserver {

class IResponseSchema {
  public:
    virtual ~IResponseSchema(){};

    virtual void generateResponse(std::string &message) = 0;
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
} // namespace echoserver