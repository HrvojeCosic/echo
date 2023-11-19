#pragma once

#include <memory>

#include "listener/server.hpp"

namespace echo {

class ResponseSchemaFactory {
  public:
    /* Returns appropriate response schema based on the schema type and arguments provided in "tokens" */
    AbstractResponseSchema createSchema(const AbstractTokens &tokens);

  private:
    /* Returns schema type based on the "choice" string (reduces number of string comparisons in the factory) */
    SchemaType getSchemaType(const std::string &choice);

    std::unique_ptr<CensoredResponseSchema> newCensoredSchema(const AbstractTokens &tokens);
};
} // namespace echo