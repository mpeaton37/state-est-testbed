pybind11_add_module(_stateest MODULE
    python_bindings.cpp
    ${PROJECT_SOURCE_DIR}/src/core/KalmanFilter.cpp
    ${PROJECT_SOURCE_DIR}/src/core/LinearGaussianModel.cpp   # add others if needed
)

target_include_directories(_stateest PRIVATE
    ${PROJECT_SOURCE_DIR}/src
    ${PROJECT_SOURCE_DIR}/src/core
)

target_link_libraries(_stateest PRIVATE
    Eigen3::Eigen
    yaml-cpp
    sqlite3
)

# Place the compiled .so directly into the Python package directory
set_target_properties(_stateest PROPERTIES
    LIBRARY_OUTPUT_DIRECTORY "${CMAKE_SOURCE_DIR}/python/stateest"
    OUTPUT_NAME "_stateest"
)

message(STATUS "Built Python module: _stateest → python/stateest/_stateest.so")
