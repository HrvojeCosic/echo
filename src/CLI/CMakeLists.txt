add_library(
    echoserver_CLI
    OBJECT
    cli_command.cpp
    tokens.cpp
)
set(ALL_OBJECT_FILES
  ${ALL_OBJECT_FILES} $<TARGET_OBJECTS:echoserver_CLI>
  PARENT_SCOPE)