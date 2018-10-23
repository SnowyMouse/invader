# Indexer executable
add_executable(invader-indexer
    src/indexer/indexer.cpp
)
target_link_libraries(invader-indexer invader)
