# Scenario executable
add_executable(invader-scenario
    src/scenario/scenario.cpp
    src/scenario/salamander_scenario.cpp
)
target_link_libraries(invader-scenario invader)
