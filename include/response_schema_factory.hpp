#pragma once

#include <memory>

#include "./response_schema.hpp"
#include "./server.hpp"

namespace echoserver {

class ResponseSchemaFactory {
  public:
    static AbstractResponseSchema createSchema(const std::string &schemaType, const uint schemaTypeOffset = 0);
};
} // namespace echoserver