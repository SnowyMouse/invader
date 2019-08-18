# Archive executable
if(${LibArchive_FOUND})
    add_executable(invader-archive
        src/archive/archive.cpp
    )
    target_include_directories(invader-archive PUBLIC ${LibArchive_INCLUDE_DIRS})
    target_link_libraries(invader-archive invader ${LibArchive_LIBRARIES})
else()
    message("A dependency is missing. invader-archive will not compile.")
endif()
