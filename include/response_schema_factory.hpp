#pragma once

#include <memory>

#include "./response_schema.hpp"
#include "./server.hpp"

namespace echoserver {

class ResponseSchemaFactory {
  public:
    /* Returns appropriate response schema based on the schema type and arguments provided in "tokens" */
    static AbstractResponseSchema createSchema(echoserverclient::AbstractTokens tokens);
};
} // namespace echoserver