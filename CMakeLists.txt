cmake_minimum_required(VERSION 3.16)
project(state_est_testbed)

set(CMAKE_CXX_STANDARD 17)

include(FetchContent)

FetchContent_Declare(
    yaml-cpp
    GIT_REPOSITORY https://github.com/jbeder/yaml-cpp.git
    GIT_TAG yaml-cpp-0.9.0 # Use a specific version tag for reliability
)

FetchContent_MakeAvailable(yaml-cpp)

find_package(Eigen3 REQUIRED)

# Add gtest
find_package(GTest REQUIRED)

add_executable(state_est_testbed
src/main.cpp
src/core/Config.cpp
src/core/KalmanFilter.cpp
src/core/LinearGaussianModel.cpp
src/core/Experiment.cpp
src/core/Database.cpp
)

target_include_directories(state_est_testbed PRIVATE src)

target_link_libraries(state_est_testbed
Eigen3::Eigen
yaml-cpp
sqlite3
)

# Test executable
add_executable(test_kalman_filter
tests/test_kalman_filter.cpp
src/core/KalmanFilter.cpp
src/core/Estimator.hpp
)

target_include_directories(test_kalman_filter PRIVATE src)

target_link_libraries(test_kalman_filter
Eigen3::Eigen
GTest::gtest_main
)

enable_testing()
add_test(NAME KalmanFilterTest COMMAND test_kalman_filter)
