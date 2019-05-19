cmake_minimum_required(VERSION 3.2)

if(CMAKE_VERSION VERSION_GREATER_EQUAL "3.12")
    message(STATUS "CMake version ${CMAKE_VERSION}")
    cmake_policy(SET CMP0074 NEW)
endif ()

if(STATIC_LINK_LIBS)
        message(STATUS "CMake STATIC_LINK_LIBS = ${STATIC_LINK_LIBS}")
        if (MSVC)
            string(REPLACE "/MD" "/MT" CMAKE_CXX_FLAGS_RELEASE ${CMAKE_CXX_FLAGS_RELEASE})
            string(REPLACE "/MD" "/MT" CMAKE_CXX_FLAGS_MINSIZEREL ${CMAKE_CXX_FLAGS_MINSIZEREL})
            string(REPLACE "/MD" "/MT" CMAKE_CXX_FLAGS_RELWITHDEBINFO ${CMAKE_CXX_FLAGS_RELWITHDEBINFO})
            string(REPLACE "/MDd" "/MTd" CMAKE_CXX_FLAGS_DEBUG ${CMAKE_CXX_FLAGS_DEBUG})
        endif ()
        set(GTEST_LINKED_AS_SHARED_LIBRARY 0)
endif ()

enable_testing()
find_package(GTest REQUIRED)

message(STATUS "GTest include: ${GTEST_INCLUDE_DIR}")

SET(TARGT_NAME test_async)

add_executable(${TARGT_NAME}  test_async.cpp)
target_include_directories(${TARGT_NAME}  PUBLIC "../include/" "../src_lib_shared/" "${CMAKE_CURRENT_BINARY_DIR}" "${CMAKE_CURRENT_BINARY_DIR}/.." ${GTEST_INCLUDE_DIR})
SET(ALL_LIBS GTest::GTest GTest::Main my_lib async)
SET(Linux_LIBS pthread stdc++fs)

if (MSVC)

        set_target_properties(${TARGT_NAME}  PROPERTIES
          CXX_STANDARD 17
          CXX_STANDARD_REQUIRED ON
          COMPILE_OPTIONS "/permissive-;/Zc:wchar_t"
        )

else()

        set_target_properties(${TARGT_NAME}  PROPERTIES
                  CXX_STANDARD 17
                  CXX_STANDARD_REQUIRED ON
                  COMPILE_OPTIONS "-Wpedantic;-Wall;-Wextra"
        )

        SET(ALL_LIBS ${ALL_LIBS} ${Linux_LIBS})

endif ()

target_link_libraries(${TARGT_NAME} ${ALL_LIBS})

