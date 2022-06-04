set(NAME page_test)
project(${NAME})


aux_source_directory(. TEST_SOURCE_FILES)

set(UTIL_SOURCE_FILES
        ${CMAKE_SOURCE_DIR}/src/util/lru.hpp
        )

include_directories(${CMAKE_SOURCE_DIR})



add_executable(${NAME}
        ${TEST_SOURCE_FILES}
        ${UTIL_SOURCE_FILES}
        )