add_executable(invader-reverse-dependency
    src/reverse_dependency/reverse_dependency.cpp
)
target_link_libraries(invader-reverse-dependency invader)
