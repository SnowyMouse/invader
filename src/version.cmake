# Set things whether or not things worked
if(${IN_GIT_REPO} STREQUAL "TRUE")
    execute_process(
        COMMAND ${GIT_EXECUTABLE} --git-dir "${GIT_DIR}" rev-parse --abbrev-ref HEAD
        OUTPUT_VARIABLE INVADER_GIT_BRANCH
    )
    execute_process(
        COMMAND ${GIT_EXECUTABLE} --git-dir "${GIT_DIR}" rev-parse --short HEAD
        OUTPUT_VARIABLE INVADER_GIT_COMMIT
    )
    execute_process(
        COMMAND ${GIT_EXECUTABLE} --git-dir "${GIT_DIR}" rev-list --count HEAD
        OUTPUT_VARIABLE INVADER_GIT_COMMIT_COUNT
    )
    string(STRIP "${INVADER_GIT_COMMIT_COUNT}" INVADER_GIT_COMMIT_COUNT)
    string(STRIP "${INVADER_GIT_COMMIT}" INVADER_GIT_COMMIT)
    set(INVADER_FULL_VERSION_TEXT "${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR}.${PROJECT_VERSION_PATCH}.r${INVADER_GIT_COMMIT_COUNT}.${INVADER_GIT_COMMIT}")
else()
    set(INVADER_FULL_VERSION_TEXT "${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR}.${PROJECT_VERSION_PATCH}.unknown")
endif()

# Overwrite version_str.hpp
file(WRITE "${OUT_FILE}" "#define INVADER_VERSION_STRING \"${INVADER_FULL_VERSION_TEXT}\"\n")
