cmake_minimum_required(VERSION 3.16)
project(state_est_testbed)


set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)


# ─── FetchContent / dependencies ────────────────────────────────
include(FetchContent)

FetchContent_Declare(
    yaml-cpp
    GIT_REPOSITORY https://github.com/jbeder/yaml-cpp.git
    GIT_TAG        yaml-cpp-0.9.0
)
FetchContent_MakeAvailable(yaml-cpp)

find_package(Eigen3 REQUIRED)
find_package(GTest REQUIRED)
# ─── NEW: pybind11 ──────────────────────────────────────────────
# Option A: Fetch as submodule (recommended for reproducibility)
#   git submodule add https://github.com/pybind/pybind11 extern/pybind11
#   git submodule update --init --recursive

# Option B: FetchContent (no git submodule needed)
FetchContent_Declare(
    pybind11
    GIT_REPOSITORY https://github.com/pybind/pybind11.git
    GIT_TAG        v2.13.1          # or latest stable tag
)
FetchContent_MakeAvailable(pybind11)

# ─── Your existing executables ──────────────────────────────────
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

# Your tests (unchanged)
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

# Integration test
add_test(NAME IntegrationTest COMMAND state_est_testbed ${CMAKE_SOURCE_DIR}/configs/test.yaml)

add_subdirectory(python_bindings)


## ─── NEW: Python module ─────────────────────────────────────────
#pybind11_add_module(kalman_py                  # → import kalman_py
#    src/python_bindings.cpp                    # your new file
#)
#
#target_include_directories(kalman_py PRIVATE
#    src
#    ${EIGEN3_INCLUDE_DIR}                      # usually not needed if find_package worked
#)
#
#target_link_libraries(kalman_py PRIVATE
#    Eigen3::Eigen
#    # yaml-cpp   ← only if you want to expose config loading to Python
#    # sqlite3    ← same
#)
#
## Optional: nicer name, output to project root or python/ folder
#set_target_properties(kalman_py PROPERTIES
#    OUTPUT_NAME "kalman"                       # → import kalman
#    LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/python"
#)