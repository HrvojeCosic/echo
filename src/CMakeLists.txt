add_subdirectory(CLI)
add_subdirectory(client)
add_subdirectory(listener)
add_subdirectory(socket)

add_library(echoserver STATIC ${ALL_OBJECT_FILES})

set(ECHOSERVER_LIBS
        echoserver_CLI
        echoserver_client
        echoserver_listener
        echoserver_socket
)

add_executable(
    dispatcher
    listener/dispatcher_main.cpp
)

add_executable(
    echo_client
    client/client_main.cpp
)

target_link_libraries(dispatcher PRIVATE ${ECHOSERVER_LIBS})
target_link_libraries(echo_client PRIVATE ${ECHOSERVER_LIBS})