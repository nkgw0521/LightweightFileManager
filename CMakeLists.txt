cmake_minimum_required(VERSION 3.24)
project(LightweightFileManager VERSION 0.2.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_AUTOMOC ON)

find_package(Qt6 6.4 REQUIRED COMPONENTS Widgets Concurrent)
find_program(CARGO_EXECUTABLE cargo REQUIRED)

if(WIN32)
    set(RUST_TARGET_DIR "${CMAKE_BINARY_DIR}/cargo-target")
    set(RUST_LIBRARY "${RUST_TARGET_DIR}/release/fm_core.lib")
else()
    set(RUST_TARGET_DIR "${CMAKE_BINARY_DIR}/cargo-target")
    set(RUST_LIBRARY "${RUST_TARGET_DIR}/release/libfm_core.a")
endif()
set(RUST_BRIDGE_HEADER "${RUST_TARGET_DIR}/cxxbridge/fm-core/src/lib.rs.h")

file(GLOB_RECURSE RUST_SOURCES CONFIGURE_DEPENDS
    "${CMAKE_SOURCE_DIR}/crates/fm-core/src/*.rs"
    "${CMAKE_SOURCE_DIR}/crates/fm-core/Cargo.toml"
    "${CMAKE_SOURCE_DIR}/Cargo.toml")

add_custom_command(
    OUTPUT "${RUST_LIBRARY}" "${RUST_BRIDGE_HEADER}"
    COMMAND "${CMAKE_COMMAND}" -E env "CARGO_TARGET_DIR=${RUST_TARGET_DIR}"
            "${CARGO_EXECUTABLE}" build --manifest-path "${CMAKE_SOURCE_DIR}/Cargo.toml"
            --release --package fm-core
    DEPENDS ${RUST_SOURCES} "${CMAKE_SOURCE_DIR}/crates/fm-core/build.rs"
    WORKING_DIRECTORY "${CMAKE_SOURCE_DIR}"
    COMMENT "Building Rust file system core"
    VERBATIM)
add_custom_target(fm_core_build DEPENDS "${RUST_LIBRARY}" "${RUST_BRIDGE_HEADER}")

qt_add_executable(lightweight-file-manager
    src/main.cpp
)
qt_add_resources(lightweight-file-manager app_icon
    PREFIX "/icons"
    FILES src/app.ico
)
if(WIN32)
    set(LIGHTWEIGHT_FILE_MANAGER_OUTPUT_DIR "${CMAKE_BINARY_DIR}/app" CACHE PATH "Application output directory")
    target_sources(lightweight-file-manager PRIVATE src/app.rc)
    set_target_properties(lightweight-file-manager PROPERTIES
        WIN32_EXECUTABLE ON
        RUNTIME_OUTPUT_DIRECTORY "${LIGHTWEIGHT_FILE_MANAGER_OUTPUT_DIR}")
endif()
add_dependencies(lightweight-file-manager fm_core_build)
target_include_directories(lightweight-file-manager PRIVATE "${RUST_TARGET_DIR}/cxxbridge")
target_link_libraries(lightweight-file-manager PRIVATE Qt6::Widgets Qt6::Concurrent "${RUST_LIBRARY}")
if(WIN32)
    target_link_libraries(lightweight-file-manager PRIVATE ws2_32 userenv bcrypt advapi32 ntdll)
elseif(UNIX)
    find_package(Threads REQUIRED)
    target_link_libraries(lightweight-file-manager PRIVATE Threads::Threads ${CMAKE_DL_LIBS} m)
endif()

