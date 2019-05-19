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
endif ()

SET(TARGT_NAME async_cli)
SET(ALL_SOURCE main.cpp)
SET(ALL_INCLUDE "../include/" "../src_lib_shared/" "${CMAKE_CURRENT_BINARY_DIR}" "${CMAKE_CURRENT_BINARY_DIR}/..")
SET(ALL_LIBS my_lib async)
SET(Linux_LIBS pthread)

add_executable(${TARGT_NAME} ${ALL_SOURCE})
target_include_directories(${TARGT_NAME} PUBLIC ${ALL_INCLUDE})

if (MSVC)

    message(STATUS "MSVC")

    set_target_properties(${TARGT_NAME} PROPERTIES
      CXX_STANDARD 17
      CXX_STANDARD_REQUIRED ON
      COMPILE_OPTIONS "/permissive-;/Zc:wchar_t"
    )

    add_library(${TARGT_NAME}.analyze OBJECT ${ALL_SOURCE})
    target_include_directories(${TARGT_NAME}.analyze PUBLIC ${ALL_INCLUDE})
        set_target_properties(${TARGT_NAME}.analyze PROPERTIES
      CXX_STANDARD 17
      CXX_STANDARD_REQUIRED ON
      COMPILE_OPTIONS "/c;/permissive-;/Zc:wchar_t;/analyze:only;/analyze:pluginEspXEngine.dll"
    )
    set_target_properties(${TARGT_NAME}.analyze PROPERTIES EXCLUDE_FROM_ALL TRUE)

    set(PVS_Studio_PATH $ENV{PVS_Studio_PATH})
    if(PVS_Studio_PATH)
            include("${PVS_Studio_PATH}/PVS-Studio.cmake")
            pvs_studio_add_target(TARGET ${TARGT_NAME}.analyzePVS ALL
                                  OUTPUT FORMAT errorfile
                                  ANALYZE ${TARGT_NAME}
                                  MODE GA:1,2 OP
                                  LOG ${CMAKE_CURRENT_BINARY_DIR}/target.err
                                  BIN "${PVS_Studio_PATH}/CompilerCommandsAnalyzer.exe")
            set_target_properties(${TARGT_NAME}.analyzePVS PROPERTIES EXCLUDE_FROM_ALL TRUE)
    endif ()

else()

        set_target_properties(${TARGT_NAME} PROPERTIES
                  CXX_STANDARD 17
                  CXX_STANDARD_REQUIRED ON
                  COMPILE_OPTIONS "-Wpedantic;-Wall;-Wextra"
        )

        SET(ALL_LIBS ${ALL_LIBS} ${Linux_LIBS})
endif ()


#set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS}" " /VERBOSE:LIB")

target_link_libraries(${TARGT_NAME} ${ALL_LIBS})

