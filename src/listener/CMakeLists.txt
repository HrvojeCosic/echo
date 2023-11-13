add_library(
    echoserver_listener
    OBJECT
    dispatcher.cpp
    listener.cpp
    response_schema.cpp
    response_schema_factory.cpp
    server.cpp
)
set(ALL_OBJECT_FILES
  ${ALL_OBJECT_FILES} $<TARGET_OBJECTS:echoserver_listener>
  PARENT_SCOPE)