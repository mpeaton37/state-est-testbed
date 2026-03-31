cmake_minimum_required(VERSION 3.18)
project(state_est_testbed LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_POSITION_INDEPENDENT_CODE ON)

# ──────────────────────────────────────────────────────────────
# Dependencies
# ──────────────────────────────────────────────────────────────
include(FetchContent)

FetchContent_Declare(
    yaml-cpp
    GIT_REPOSITORY https://github.com/jbeder/yaml-cpp.git
    GIT_TAG yaml-cpp-0.9.0
)
FetchContent_MakeAvailable(yaml-cpp)

find_package(Eigen3 REQUIRED)
find_package(pybind11 REQUIRED)
find_package(GTest REQUIRED)

# ──────────────────────────────────────────────────────────────
# Main C++ Executable
# ──────────────────────────────────────────────────────────────
add_executable(state_est_testbed
    src/main.cpp
    src/core/Config.cpp
    src/core/Database.cpp
    src/core/Experiment.cpp
    src/core/KalmanFilter.cpp
    src/core/EstimatorFactory.cpp
    src/core/UnscentedKalmanFilter.cpp
    src/core/ExtendedKalmanFilter.cpp
    src/core/ParticleFilter.cpp
    src/core/LinearGaussianModel.cpp
)

target_include_directories(state_est_testbed PRIVATE src)
target_link_libraries(state_est_testbed PRIVATE Eigen3::Eigen yaml-cpp sqlite3)

# ──────────────────────────────────────────────────────────────
# Tests
# ──────────────────────────────────────────────────────────────
add_executable(test_kalman_filter
    tests/test_kalman_filter.cpp
    src/core/KalmanFilter.cpp
)

target_include_directories(test_kalman_filter PRIVATE src)
target_link_libraries(test_kalman_filter PRIVATE Eigen3::Eigen GTest::gtest_main)

enable_testing()
add_test(NAME KalmanFilterTest COMMAND test_kalman_filter)

# ──────────────────────────────────────────────────────────────
# Python Bindings
# ──────────────────────────────────────────────────────────────
add_subdirectory(python_bindings)

message(STATUS "Python module _stateest will be placed in: python/stateest/_stateest.so")


