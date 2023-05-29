file(GLOB_RECURSE project_sources ${PROJECT_SOURCE_DIR}/src/*/*.cpp)
set(PROJECT_LIB ${PROJECT_NAME}_lib)
add_library(${PROJECT_LIB} SHARED ${project_sources})
target_link_libraries(${PROJECT_LIB} uring)

# add executable
add_executable(main main.cpp)
add_executable(echo_server echo_server_eg.cpp)
target_link_libraries(echo_server ${PROJECT_LIB})
# target_compile_options(echo_server PRIVATE "-pg")

# murmur3
# file(GLOB_RECURSE murmur3_sources
#         ${PROJECT_SOURCE_DIR}/third_party/murmur3/*.cpp ${PROJECT_SOURCE_DIR}/third_party/murmur3/*.h)
# add_library(thirdparty_murmur3 SHARED ${murmur3_sources})
# target_link_libraries(bustub_shared thirdparty_murmur3)