include_directories(${CMAKE_SOURCE_DIR}/third_party/googletest/googletest/include/)
include_directories(${CMAKE_SOURCE_DIR}/include)

add_executable(t1 t1.cpp)
target_link_libraries(t1 gtest gtest_main memort_pool)
add_executable(t2 t2.cpp)
target_link_libraries(t2 gtest gtest_main memort_pool)