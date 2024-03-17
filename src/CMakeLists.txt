file(GLOB SOURCE_FILES ${CMAKE_SOURCE_DIR}/src/*.cpp)
include_directories(${CMAKE_SOURCE_DIR}/include)
include_directories(${CMAKE_SOURCE_DIR}/third_party/)
add_library(memort_pool ${SOURCE_FILES})
