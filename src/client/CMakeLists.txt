add_library(
    echoserver_client
    OBJECT
    client.cpp
)
set(ALL_OBJECT_FILES
  ${ALL_OBJECT_FILES} $<TARGET_OBJECTS:echoserver_client>
  PARENT_SCOPE)