cmake_minimum_required(VERSION 3.15)
project(EpollDemo)

set(CMAKE_CXX_STANDARD 17)

SET(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -O3")
SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -O3")

include_directories(${CMAKE_SOURCE_DIR}
        eventBus/
        libevent/include/
        EventBus/include/)

add_subdirectory(eventBus)

add_subdirectory(EventBus)

add_subdirectory(libevent)

add_executable(EpollDemo main.cpp ThreadPool.h hello-world.cpp hello-world.h)

set(libevent-lib
        event
        event_core
        event_extra
        event_openssl
        event_pthreads)


target_link_libraries(EpollDemo pthread eventBus ${libevent-lib} EventBus)

target_compile_options(EpollDemo PUBLIC
        -Wall -pedantic
        -Wnon-virtual-dtor
        -Werror
        -Wno-error=deprecated-declarations
        )