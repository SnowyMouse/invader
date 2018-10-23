# Build executable
add_executable(invader-build
    src/build/build.cpp
    src/build/build_workload.cpp
)
target_link_libraries(invader-build invader)
