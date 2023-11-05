add_library(
    echoclientserver
    server.cpp
    client.cpp
    cli_command.cpp
    client.cpp
    inet_socket.cpp
    response_schema.cpp
    response_schema_factory.cpp
    socket_factory.cpp
    tokens.cpp
    unix_socket.cpp
)

add_executable(
    echo_server
    server_main.cpp
)

add_executable(
    echo_client
    client_main.cpp
)

target_link_libraries(echo_client PRIVATE echoclientserver)
target_link_libraries(echo_server PRIVATE echoclientserver)