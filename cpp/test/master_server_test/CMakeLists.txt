set(NAME master_server)
project(${NAME})


aux_source_directory(. TEST_SOURCE_FILES)

include_directories(${CMAKE_SOURCE_DIR})
include_directories(${GLOBAL_BRPC_INCLUDE_DIR})

set(PROTOBUF_SOURCE_FILES
        ${CMAKE_SOURCE_DIR}/proto/out/gfs.pb.h
        ${CMAKE_SOURCE_DIR}/proto/out/gfs.pb.cc
        )

set(MASTER_SERVER_SOURCE_FILES
        ${CMAKE_SOURCE_DIR}/src/chunk/chunk_client.h
        ${CMAKE_SOURCE_DIR}/src/chunk/chunk_client.cc
        ${CMAKE_SOURCE_DIR}/src/master/master_server.h
        ${CMAKE_SOURCE_DIR}/src/master/master_server.cc
        )

set(UTIL_SOURCE_FILES
        ${CMAKE_SOURCE_DIR}/src/util/conf.h
        ${CMAKE_SOURCE_DIR}/src/util/lock.h
        ${CMAKE_SOURCE_DIR}/src/util/lock.cc
        ${CMAKE_SOURCE_DIR}/src/util/state_code.h
        ${CMAKE_SOURCE_DIR}/src/util/state_code.cc
        )


message("USAGE: ${MASTER_SERVER_SOURCE_FILES}")
message("USAGE: ${UTIL_SOURCE_FILES}")


add_executable(${NAME}
        ${TEST_SOURCE_FILES}
        ${PROTOBUF_SOURCE_FILES}
        ${MASTER_SERVER_SOURCE_FILES}
        ${UTIL_SOURCE_FILES}
        )

target_link_libraries(${NAME}
        ${GLOBAL_BRPC_LIBRARY_BRPC_A}
        ${GLOBAL_GFLAGS_LIBRARY_LIB}
        ${OPENSSL_CRYPTO_LIBRARY}
        ${OPENSSL_SSL_LIBRARY}
        ${GLOBAL_LEVELDB_LIBRARY_LIB}
        pthread
        protobuf
        dl
        )