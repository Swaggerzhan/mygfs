set(NAME init)
project(${NAME})


include_directories(${GLOBAL_BRPC_INCLUDE_DIR})



aux_source_directory(. INIT_TEST_FILES)


add_executable(${NAME}
        ${INIT_TEST_FILES}
        )

target_link_libraries(${NAME}
        ${GLOBAL_BRPC_LIBRARY_BRPC_A}
        ${GLOBAL_GFLAGS_LIBRARY_LIB}
        ${OPENSSL_CRYPTO_LIBRARY}
        ${OPENSSL_SSL_LIBRARY}
        pthread
        protobuf
        )

