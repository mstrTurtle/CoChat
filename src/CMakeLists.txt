if(CHATROOM_BUILD_SERVER)
    add_subdirectory(server)
endif()

if(CHATROOM_BUILD_SIMPLE_CLIENT)
    add_subdirectory(simple-client)
endif()

if(CHATROOM_BUILD_COMPLEX_CLIENT)
    add_subdirectory(complex-client)
endif()

# add_subdirectory(server)
# add_subdirectory(simple-client)
# add_subdirectory(complex-client)