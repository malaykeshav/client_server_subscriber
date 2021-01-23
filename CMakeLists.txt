cmake_minimum_required(VERSION 3.0.0)
project(ClientServerSubscriber VERSION 0.1.0)

set(CMAKE_CXX_STANDARD 14) 
set(CMAKE_CXX_STANDARD_REQUIRED ON) 
set(CMAKE_CXX_FLAGS ${CMAKE_CXX_FLAGS} "-std=c++14 -pthread")

set(CPACK_PROJECT_NAME ${PROJECT_NAME}) 
set(CPACK_PROJECT_VERSION ${PROJECT_VERSION})

# Enable SAMPLE_SERVER or SAMPLE_CLIENT
# to compile one of the examples.
# Each example contains a main() function, so
# enable only one example, to avoid main() collision
add_definitions(
    -DSAMPLE_SERVER
#   - DSAMPLE_CLIENT
)

add_executable(Sample_Server 
               sample_server.cc 
               server/client_manager_thread.cc
               server/server.cc 
               server/subscriber.cc 
               server/subscriber_writer_thread.cc
               common/file_reader.cc 
               common/news_item.cc)

add_executable(Sample_Client 
               sample_client.cc 
               client/client.cc 
               common/file_reader.cc 
               common/news_item.cc)

target_link_libraries(Sample_Server ${CMAKE_THREAD_LIBS_INIT})