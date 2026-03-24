# python_bindings/CMakeLists.txt

pybind11_add_module(_stateest MODULE 
    python_bindings.cpp
    ${PROJECT_SOURCE_DIR}/src/core/KalmanFilter.cpp
)

target_include_directories(_stateest
    PRIVATE
        ${PROJECT_SOURCE_DIR}/src
        ${PROJECT_SOURCE_DIR}/src/core       # so it finds core/KalmanFilter.hpp etc.
)

target_link_libraries(_stateest
    PRIVATE
        Eigen3::Eigen
        # yaml-cpp      # only if bindings actually use it
        # sqlite3       # same
)
set_target_properties(_stateest PROPERTIES
    LIBRARY_OUTPUT_DIRECTORY "${CMAKE_SOURCE_DIR}/python/stateest"
    OUTPUT_NAME "_stateest"
)
## Nice-to-have: control the .so / .pyd name and where it lands
#set_target_properties(kalman PROPERTIES
#    OUTPUT_NAME "kalman"                      # → import kalman
#    LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/python"
#    POSITION_INDEPENDENT_CODE ON
#    # Optional: PDB symbol files, etc. for Windows debugging
#)
#