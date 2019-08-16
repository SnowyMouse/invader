# Set things whether or not things worked
if(${IN_GIT_REPO})
    execute_process(
        COMMAND ${GIT_EXECUTABLE} --git-dir "${CMAKE_CURRENT_SOURCE_DIR}/.git" rev-parse --abbrev-ref HEAD
        OUTPUT_VARIABLE INVADER_GIT_BRANCH
    )
    execute_process(
        COMMAND ${GIT_EXECUTABLE} --git-dir "${CMAKE_CURRENT_SOURCE_DIR}/.git" rev-parse --short HEAD
        OUTPUT_VARIABLE INVADER_GIT_COMMIT
    )
    execute_process(
        COMMAND ${GIT_EXECUTABLE} --git-dir "${CMAKE_CURRENT_SOURCE_DIR}/.git" rev-list --count HEAD
        OUTPUT_VARIABLE INVADER_GIT_COMMIT_COUNT
    )
    add_custom_command(
        OUTPUT "${CMAKE_CURRENT_BINARY_DIR}/version_str.hpp"
        COMMAND ${CMAKE_COMMAND} -P ${CMAKE_CURRENT_SOURCE_DIR}/src/version.cmake
        DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/.git/refs/heads/${INVADER_GIT_BRANCH}"
    )
    string(STRIP "${INVADER_GIT_COMMIT_COUNT}" INVADER_GIT_COMMIT_COUNT)
    string(STRIP "${INVADER_GIT_COMMIT}" INVADER_GIT_COMMIT)
    set(INVADER_FULL_VERSION_TEXT "${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR}.${PROJECT_VERSION_PATCH}.r${INVADER_GIT_COMMIT_COUNT}.${INVADER_GIT_COMMIT}")
else()
    set(INVADER_FULL_VERSION_TEXT "${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR}.${PROJECT_VERSION_PATCH}")
endif()

# Overwrite version_str.hpp
file(WRITE "${CMAKE_CURRENT_BINARY_DIR}/version_str.hpp" "#define INVADER_VERSION_STRING \"${INVADER_FULL_VERSION_TEXT}\"\n")
