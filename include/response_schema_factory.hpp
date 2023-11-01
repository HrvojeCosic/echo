#pragma once

#include <memory>

#include "./response_schema.hpp"
#include "./server.hpp"

namespace echoserver {

class ResponseSchemaFactory {
  public:
    static AbstractResponseSchema createSchema(echoserverclient::AbstractTokens tokens);
};
} // namespace echoserver