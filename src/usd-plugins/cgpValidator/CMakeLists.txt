cmake_minimum_required(VERSION 3.23)
project(CheckUpAxisValidator LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

set(REPO_ROOT "${CMAKE_CURRENT_LIST_DIR}/../../..")
set(USD_ROOT "${REPO_ROOT}/_build/usd-deps/usd")
set(PYTHON_ROOT "${REPO_ROOT}/_build/usd-deps/python")

set(Python3_ROOT_DIR "${PYTHON_ROOT}")
set(Python3_EXECUTABLE "${PYTHON_ROOT}/bin/python3")
set(Python3_INCLUDE_DIR "${PYTHON_ROOT}/include/python3.12")
set(Python3_LIBRARY "${PYTHON_ROOT}/lib/libpython3.12.so")
set(Python3_LIBRARY_RELEASE "${Python3_LIBRARY}")
set(Python3_LIBRARY_DEBUG "${Python3_LIBRARY}")
find_package(Python3 3.12 REQUIRED COMPONENTS Interpreter Development)

set(USD_INCLUDE_DIR "${USD_ROOT}/include")
set(USD_LIB_DIR "${USD_ROOT}/lib")
set(USD_PLUGIN_NAME cgpValidatorPlugin)

add_library(${USD_PLUGIN_NAME} SHARED
    validators.cpp
)

target_compile_definitions(${USD_PLUGIN_NAME}
    PRIVATE
        NOMINMAX=1
)

target_compile_options(${USD_PLUGIN_NAME} PRIVATE
    -Wno-changes-meaning
    -Wno-error=changes-meaning
    -Wno-deprecated-declarations
    -Wno-error=deprecated-declarations
)

target_include_directories(${USD_PLUGIN_NAME}
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}
        ${USD_INCLUDE_DIR}
        ${Python3_INCLUDE_DIRS}
)

target_link_directories(${USD_PLUGIN_NAME} PRIVATE ${USD_LIB_DIR} ${PYTHON_ROOT}/lib)

target_link_libraries(${USD_PLUGIN_NAME}
    PRIVATE
        ${USD_LIB_DIR}/libusd_usdValidation.so
        ${USD_LIB_DIR}/libusd_usdUtils.so
        ${USD_LIB_DIR}/libusd_usdGeomValidators.so
        ${USD_LIB_DIR}/libusd_usdGeom.so
        ${USD_LIB_DIR}/libusd_usd.so
        ${USD_LIB_DIR}/libusd_sdf.so
        ${USD_LIB_DIR}/libusd_tf.so
        ${USD_LIB_DIR}/libusd_arch.so
        ${PYTHON_ROOT}/lib/libpython3.12.so
)

# The plugin registry file is a simple JSON resource. Use the same install-root
# layout as the other repo-local USD plugins.
set(PLUG_INFO_ROOT ".")
set(PLUG_INFO_LIBRARY_PATH "../../../lib/libcgpValidatorPlugin.so")
set(PLUG_INFO_RESOURCE_PATH ".")
configure_file(
    "${CMAKE_CURRENT_SOURCE_DIR}/plugInfo.json"
    "${CMAKE_CURRENT_BINARY_DIR}/plugInfo.json"
    @ONLY
)

install(TARGETS ${USD_PLUGIN_NAME}
    LIBRARY DESTINATION lib
    RUNTIME DESTINATION bin
)

install(FILES "${CMAKE_CURRENT_BINARY_DIR}/plugInfo.json"
    DESTINATION plugins/${USD_PLUGIN_NAME}/resources
)
