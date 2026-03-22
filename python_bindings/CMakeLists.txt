# python_bindings/CMakeLists.txt

pybind11_add_module(kalman
    ../python_bindings/python_bindings.cpp               # adjust path if file is elsewhere
    # or just python_bindings.cpp if you move the .cpp file into this folder
)

target_include_directories(kalman
    PRIVATE
        ${PROJECT_SOURCE_DIR}/src
        ${PROJECT_SOURCE_DIR}/src/core       # so it finds core/KalmanFilter.hpp etc.
)

target_link_libraries(kalman
    PRIVATE
        Eigen3::Eigen
        # yaml-cpp      # only if bindings actually use it
        # sqlite3       # same
)

# Nice-to-have: control the .so / .pyd name and where it lands
set_target_properties(kalman PROPERTIES
    OUTPUT_NAME "kalman"                      # → import kalman
    LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/python"
    # Optional: PDB symbol files, etc. for Windows debugging
)

