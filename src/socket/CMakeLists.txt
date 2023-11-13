add_library(
    echoserver_socket
    OBJECT
    inet_socket.cpp
    unix_socket.cpp
    socket_factory.cpp
)
set(ALL_OBJECT_FILES
  ${ALL_OBJECT_FILES} $<TARGET_OBJECTS:echoserver_socket>
  PARENT_SCOPE)