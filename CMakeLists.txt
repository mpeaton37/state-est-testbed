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

add_executable(state_est_testbed
src/main.cpp
src/core/Config.cpp
src/core/KalmanFilter.cpp
src/core/LinearGaussianModel.cpp
src/core/Experiment.cpp
)

target_include_directories(state_est_testbed PRIVATE src)

target_link_libraries(state_est_testbed
Eigen3::Eigen
yaml-cpp
)
