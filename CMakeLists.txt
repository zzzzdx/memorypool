cmake_minimum_required(VERSION 3.10)
project(memory_pool)
enable_testing()
add_subdirectory(src)
add_subdirectory(tests)
add_subdirectory(third_party)

set(CMAKE_INSTALL_PREFIX "./memory_pool/")
install(TARGETS memort_pool
        ARCHIVE DESTINATION lib)
file(GLOB HEADER_FILES ${CMAKE_SOURCE_DIR}/include/*.h)
install(FILES ${HEADER_FILES} DESTINATION include)