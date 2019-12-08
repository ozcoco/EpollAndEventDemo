
add_library(eventBus STATIC EventBus.cpp)

target_compile_options(eventBus PUBLIC
        -Wall -pedantic
        -Wnon-virtual-dtor
        -Werror
        -Wno-error=deprecated-declarations
        )