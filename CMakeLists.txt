cmake_minimum_required(VERSION 3.16)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON) # For lsp
set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

project(cxxtemplate 
        VERSION 0.1.0
        DESCRIPTION "This is a CMake Template for C++ project"
        LANGUAGES CXX)


######################################################################################################################
# GTEST FETCH
######################################################################################################################
include(FetchContent)
# use a hack archive here
FetchContent_Declare(
        googletest
        URL https://github.com/google/googletest/archive/03597a01ee50ed33e9dfd640b249b4be3799d395.zip
        DOWNLOAD_EXTRACT_TIMESTAMP TRUE
)
# For Windows: Prevent overriding the parent project's compiler/linker settings
set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(googletest)


######################################################################################################################
# CTEST SETUP
######################################################################################################################
# uncomment will enable more functions
# include(CTest)

# basic CTtest
enable_testing()


######################################################################################################################
# COMPILER SETUP
######################################################################################################################

# Compiler flags.
# for clang
# set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fPIC -fcoroutines-ts -Wall -Wextra -Werror")
# for g++
# set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fPIC -fcoroutines -Wall -Wextra -Werror")
# set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fcoroutines -Wall -Wextra -Werror -pg")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fcoroutines -Wall -Wextra -Werror")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wno-unused-parameter -Wno-attributes") #TODO: remove
set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -O0 -ggdb -fsanitize=address -fno-omit-frame-pointer")
set(CMAKE_EXE_LINKER_FLAGS  "${CMAKE_EXE_LINKER_FLAGS} -fPIC")
set(CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS} -fPIC")
set(CMAKE_STATIC_LINKER_FLAGS "${CMAKE_STATIC_LINKER_FLAGS} -fPIC")

# set(GCC_COVERAGE_LINK_FLAGS    "-fPIC")
message(STATUS "CMAKE_CXX_FLAGS: ${CMAKE_CXX_FLAGS}")
message(STATUS "CMAKE_CXX_FLAGS_DEBUG: ${CMAKE_CXX_FLAGS_DEBUG}")
message(STATUS "CMAKE_EXE_LINKER_FLAGS: ${CMAKE_EXE_LINKER_FLAGS}")
message(STATUS "CMAKE_SHARED_LINKER_FLAGS: ${CMAKE_SHARED_LINKER_FLAGS}")
message(STATUS "CMAKE_STATIC_LINKER_FLAGS: ${CMAKE_STATIC_LINKER_FLAGS}")

# Output directory.
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

# Includes.
set(PROJECT_SRC_INCLUDE_DIR  ${PROJECT_SOURCE_DIR}/src/include)
set(PROJECT_TEST_INCLUDE_DIR ${PROJECT_SOURCE_DIR}/test/include)
set(PROJECT_THIRD_PARTY_INCLUDE_DIR ${PROJECT_SOURCE_DIR}/thirdparty)
include_directories(${PROJECT_SRC_INCLUDE_DIR} ${PROJECT_TEST_INCLUDE_DIR} ${PROJECT_THIRD_PARTY_INCLUDE_DIR})

add_subdirectory(src)
add_subdirectory(test)


# set(CPACK_PROJECT_NAME ${PROJECT_NAME})
# set(CPACK_PROJECT_VERSION ${PROJECT_VERSION})
# include(CPack)