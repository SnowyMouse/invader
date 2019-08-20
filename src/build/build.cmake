# Build executable
add_executable(invader-build
    src/build/build.cpp
)
target_link_libraries(invader-build invader)
