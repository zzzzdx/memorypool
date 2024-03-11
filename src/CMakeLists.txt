file(GLOB SOURCE_FILES ${CMAKE_SOURCE_DIR}/src/*.cpp)
include_directories(${CMAKE_SOURCE_DIR}/include)
add_library(memort_pool ${SOURCE_FILES})
