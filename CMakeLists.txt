PROJECT(GraphDB)
CMAKE_MINIMUM_REQUIRED(VERSION 2.6) 

set(CMAKE_C_COMPILER "gcc")
set(CMAKE_CXX_COMPILER "g++")

set(CMAKE_C_FLAGS "-Wall -g -O0 -I./include ")
set(CMAKE_CXX_FLAGS "-g -O0 -std=c++14 -I./include")

set(MICRO_TEST_LIST
    test_multiget
    test_tailing
    )

set(TEST_LIST
    test_simple
    test_strawman_join
    test_graphdb
    )

include_directories(
    src
    src/core
    )

    #../rocksdb/include

# add_library(${staticlib}
#     ${CMAKE_SOURCE_DIR}/lib/librocksdb.a)

set(mylib
    ${CMAKE_SOURCE_DIR}/lib/librocksdb.a
    stdc++
    snappy
    bz2 
    z
    m
    pthread
    dl
    gtest
    zstd)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY bin/micro-test/)

foreach (TEST_NAME IN LISTS MICRO_TEST_LIST)

    add_executable( ${TEST_NAME} micro-test/${TEST_NAME}.cpp )

    target_link_libraries( 
        ${TEST_NAME}
        ${mylib}
        )

endforeach()

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY bin/test/)

foreach (TEST_NAME IN LISTS TEST_LIST)

    add_executable( ${TEST_NAME} test/${TEST_NAME}.cpp )

    target_link_libraries( 
        ${TEST_NAME}
        ${mylib}
        )

endforeach()


