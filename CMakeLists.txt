cmake_minimum_required(VERSION 3.15)
project(DiscreteX VERSION 1.0.0 LANGUAGES CXX)

include(GNUInstallDirs)
include(CMakePackageConfigHelpers)

# Interface Library
add_library(discretex INTERFACE)
add_library(DiscreteX::DiscreteX ALIAS discretex)

target_compile_features(discretex INTERFACE cxx_std_20)

target_include_directories(discretex INTERFACE
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>
)

# Installation Rules
install(TARGETS discretex
    EXPORT DiscreteXTargets
    INCLUDES DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
)

install(DIRECTORY include/discretex
    DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
)

install(EXPORT DiscreteXTargets
    NAMESPACE DiscreteX::
    DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/DiscreteX
)

write_basic_package_version_file(
    "${CMAKE_CURRENT_BINARY_DIR}/DiscreteXConfigVersion.cmake"
    VERSION ${PROJECT_VERSION}
    COMPATIBILITY SameMajorVersion
)

configure_package_config_file(
    "${CMAKE_CURRENT_SOURCE_DIR}/cmake/DiscreteXConfig.cmake.in"
    "${CMAKE_CURRENT_BINARY_DIR}/DiscreteXConfig.cmake"
    INSTALL_DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/DiscreteX
)

install(FILES
    "${CMAKE_CURRENT_BINARY_DIR}/DiscreteXConfig.cmake"
    "${CMAKE_CURRENT_BINARY_DIR}/DiscreteXConfigVersion.cmake"
    DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/DiscreteX
)

# Testing and Examples Options
option(DISCRETEX_BUILD_TESTS "Build DiscreteX test suite" ON)
option(DISCRETEX_BUILD_EXAMPLES "Build DiscreteX examples" ON)

if(DISCRETEX_BUILD_TESTS)
    enable_testing()
    add_executable(run_tests tests/test_main.cpp)
    target_link_libraries(run_tests PRIVATE discretex)
    add_test(NAME CoreTests COMMAND run_tests)
endif()

if(DISCRETEX_BUILD_EXAMPLES)
    file(GLOB EXAMPLE_SOURCES CONFIGURE_DEPENDS "examples/*.cpp")
    foreach(EXAMPLE_SRC ${EXAMPLE_SOURCES})
        get_filename_component(EXAMPLE_NAME ${EXAMPLE_SRC} NAME_WE)
        add_executable(${EXAMPLE_NAME} ${EXAMPLE_SRC})
        target_link_libraries(${EXAMPLE_NAME} PRIVATE discretex)
    endforeach()
endif()
