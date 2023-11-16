#pragma once

#include <memory>

#include "listener/server.hpp"

namespace echo {

class ResponseSchemaFactory {
  public:
    /* Returns appropriate response schema based on the schema type and arguments provided in "tokens" */
    static AbstractResponseSchema createSchema(AbstractTokens tokens);
};
} // namespace echo