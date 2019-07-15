# Resource executable
if(${Python3_FOUND})
    add_executable(invader-resource
        src/resource/resource.cpp
        ${CMAKE_CURRENT_BINARY_DIR}/resource_list.cpp
    )

    add_custom_command(OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/resource_list.cpp
        COMMAND ${Python3_EXECUTABLE} ${CMAKE_CURRENT_SOURCE_DIR}/src/resource/list/generator.py ${CMAKE_CURRENT_SOURCE_DIR}/src/resource/list/bitmaps.tag_indices ${CMAKE_CURRENT_SOURCE_DIR}/src/resource/list/sounds.tag_indices ${CMAKE_CURRENT_SOURCE_DIR}/src/resource/list/loc.tag_indices ${CMAKE_CURRENT_BINARY_DIR}/resource_list.cpp

        DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/src/resource/list/generator.py ${CMAKE_CURRENT_SOURCE_DIR}/src/resource/list/bitmaps.tag_indices ${CMAKE_CURRENT_SOURCE_DIR}/src/resource/list/sounds.tag_indices ${CMAKE_CURRENT_SOURCE_DIR}/src/resource/list/loc.tag_indices
    )

    target_link_libraries(invader-resource invader)
else()
    message("A dependency is missing. invader-resource will not compile.")
endif()
