cmake_minimum_required(VERSION 3.2)

if(STATIC_LINK_LIBS)
        message(STATUS "CMake STATIC_LINK_LIBS = ${STATIC_LINK_LIBS}")            
        if (MSVC)
            string(REPLACE "/MD" "/MT" CMAKE_CXX_FLAGS_RELEASE ${CMAKE_CXX_FLAGS_RELEASE})
            string(REPLACE "/MD" "/MT" CMAKE_CXX_FLAGS_MINSIZEREL ${CMAKE_CXX_FLAGS_MINSIZEREL})
            string(REPLACE "/MD" "/MT" CMAKE_CXX_FLAGS_RELWITHDEBINFO ${CMAKE_CXX_FLAGS_RELWITHDEBINFO})
            string(REPLACE "/MDd" "/MTd" CMAKE_CXX_FLAGS_DEBUG ${CMAKE_CXX_FLAGS_DEBUG})
        endif ()
endif ()

SET(TARGT_NAME async)

add_library(${TARGT_NAME} SHARED async.cpp)
target_include_directories(${TARGT_NAME} PUBLIC "../include/" "${CMAKE_CURRENT_BINARY_DIR}" "${CMAKE_CURRENT_BINARY_DIR}/.." )

if (MSVC)

    message(STATUS "MSVC")

	set_target_properties(${TARGT_NAME} PROPERTIES
        CXX_STANDARD 17
        CXX_STANDARD_REQUIRED ON
        COMPILE_OPTIONS "/permissive-;/Zc:wchar_t;-Dshared_EXPORTS"
    )

else()

	set_target_properties(${TARGT_NAME} PROPERTIES
        CXX_STANDARD 17
		CXX_STANDARD_REQUIRED ON
		COMPILE_OPTIONS "-Wpedantic;-Wall;-Wextra"
	)

endif ()

install(TARGETS ${TARGT_NAME}
        LIBRARY
          DESTINATION lib
        PUBLIC_HEADER
          DESTINATION include
       )