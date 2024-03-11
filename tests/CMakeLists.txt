include_directories(${CMAKE_SOURCE_DIR}/third_party/googletest/googletest/include/gtest)
include_directories(${CMAKE_SOURCE_DIR}/include)

add_executable(t1 t1.cpp)
target_link_libraries(t1 gtest gtest_main memort_pool)