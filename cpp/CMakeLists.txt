cmake_minimum_required(VERSION 2.16)
set(NAME mygfs)
project(${NAME})

aux_source_directory(src GLOBAL_SOURCE_FILES)
aux_source_directory(proto GLOBAL_PROTO_FILES)

# threads & protobuf
include(FindThreads)
include(FindProtobuf)

# brpc
find_path(GLOBAL_BRPC_INCLUDE_DIR NAMES brpc/server.h PATHS /root/Github/incubator-brpc/output/include)
find_library(GLOBAL_BRPC_LIBRARY_BRPC_A NAMES libbrpc.a PATHS /root/Github/incubator-brpc/output/lib)

# gflags
find_path(GLOBAL_GFLAGS_INCLUDE_DIR NAMES gflags/gflags.h)
find_library(GLOBAL_GFLAGS_LIBRARY_LIB NAMES gflags)

# openssl
find_package(OpenSSL)

# leveldb
find_path(GLOBAL_LEVELDB_INCLUDE_DIR NAMES leveldb/db.h)
find_library(GLOBAL_LEVELDB_LIBRARY_LIB NAMES leveldb)


message("***********************FIND*************************")

message("INCLUDE PATH INFO: ${CMAKE_INCLUDE_PATH}")
message("LIBRARY PATH INFO: ${CMAKE_LIBRARY_PATH}")

message("FIND: ${GLOBAL_BRPC_INCLUDE_DIR}")
message("FIND: ${GLOBAL_BRPC_LIBRARY_DIR}")

message("FIND: ${GLOBAL_GFLAGS_INCLUDE_DIR}")
message("FIND: ${GLOBAL_GFLAGS_LIBRARY_LIB}")

message("FIND: ${GLOBAL_LEVELDB_INCLUDE_DIR}")
message("FIND: ${GLOBAL_LEVELDB_LIBRARY_LIB}")

message("****************************************************")


include_directories(${GLOBAL_BRPC_INCLUDE_DIR})
include_directories(${CMAKE_SOURCE_DIR})


add_subdirectory(test/init)
add_subdirectory(test/chunk_server_test)
add_subdirectory(test/chunk_client_test)
add_subdirectory(test/master_server_test)
add_subdirectory(test/master_client_test)

aux_source_directory(proto/out PROTOBUF_SOURCE_FILES)
aux_source_directory(src/master MASTER_SOURCE_FILES)
aux_source_directory(src/chunk CHUNK_SOURCE_FILES)
aux_source_directory(src/util UTIL_SOURCE_FILES)
aux_source_directory(src/gfs GFS_SOURCE_FILES)

set(GLOBAL_PROTOBUF_SOURCE_FILES
        ${PROJECT_SOURCE_DIR}/proto/out/gfs.pb.h
        ${PROJECT_SOURCE_DIR}/proto/out/gfs.pb.cc
        )

set(GLOBAL_CHUNK_SERVER_SOURCE_FILES
        ${PROJECT_SOURCE_DIR}/src/chunk/chunk_server.h
        ${PROJECT_SOURCE_DIR}/src/chunk/chunk_server.cc
        )

set(GLOBAL_UTIL_SOURCE_FILES
        ${PROJECT_SOURCE_DIR}/src/util/conf.h
        ${PROJECT_SOURCE_DIR}/src/util/lock.h
        ${PROJECT_SOURCE_DIR}/src/util/lock.cc
        ${PROJECT_SOURCE_DIR}/src/util/state_code.h
        ${PROJECT_SOURCE_DIR}/src/util/state_code.cc
        )



add_library(${NAME}
        STATIC
        ${PROTOBUF_SOURCE_FILES}
        ${MASTER_SOURCE_FILES}
        ${CHUNK_SOURCE_FILES}
        ${UTIL_SOURCE_FILES}
        ${GFS_SOURCE_FILES}
        )




