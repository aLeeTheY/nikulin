cmake_minimum_required(VERSION 3.16)

project(NikulinTasks
    VERSION 1.2.0.0
    LANGUAGES C CXX
)

# C17 & CPP20 use
set(CMAKE_C_STANDARD 17)
set(CMAKE_C_STANDARD_REQUIRED ON)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

if (MSVC)
    add_compile_options(
        /Zc:__cplusplus

        /D_CRT_SECURE_NO_WARNINGS
        /D_UTF8

        /DFMT_SHARED
        /DSPDLOG_COMPILED_LIB
        /DSPDLOG_FMT_EXTERNAL
        /DSPDLOG_SHARED_LIB
        /DSPDLOG_FWRITE_UNLOCKED

        /DBOOST_ATOMIC_NO_LIB
        /DBOOST_ATOMIC_DYN_LINK
        /DBOOST_FILESYSTEM_NO_LIB
        /DBOOST_FILESYSTEM_DYN_LINK
        /DBOOST_PROGRAM_OPTIONS_NO_LIB
        /DBOOST_PROGRAM_OPTIONS_DYN_LINK
    )
endif()

# all warnings as errors
# https://stackoverflow.com/questions/2368811/how-to-set-warning-level-in-cmake
# https://cmake.org/cmake/help/latest/command/add_compile_options.html
if (MSVC)
    # warning all warnings as errors
    # add_compile_options(/Wall /WX)
    add_compile_options(/W4 /WX)
else()
    # lots of warnings and all warnings as errors
    add_compile_options(-Wall -Wextra -Wpedantic -Werror -Wno-deprecated -Wno-overlength-strings)
    if (CMAKE_BUILD_TYPE MATCHES "^[Rr]elease")
        # эти предупреждения возникают только в release версии, и относятся к системным заголовочным файлам
        # их нужно выключить, иначе сборка GeneratorResearch не работает
        add_compile_options(-Wno-stringop-overflow -Wno-array-bounds -Wno-restrict)
    endif()
endif()

# control where the static and shared libraries are built so that on windows
# we don't need to tinker with the path to run the executable
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY "${PROJECT_BINARY_DIR}")
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY "${PROJECT_BINARY_DIR}")
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${PROJECT_BINARY_DIR}")

add_custom_target (AStyle SOURCES .astylerc)

# option(BUILD_SHARED_LIBS "Build using shared libraries" ON)

# search and load required libraries
set(Boost_NO_WARN_NEW_VERSIONS ON)
find_package(Boost 1.76.0 COMPONENTS program_options filesystem REQUIRED)
#include_directories(${Boost_INCLUDE_DIR})

find_package(OpenCL REQUIRED)
include_directories(${OpenCL_INCLUDE_DIR})

# если венда, то используются библиотеки из vcpkg, 
# под linux pthread.h и getopt.h встроены в систему и не нужнаются в подгрузке из vcpkg
if (WIN32)
    find_package(PThreads4W REQUIRED)
    include_directories(${PThreads4W_INCLUDE_DIR})

    find_path(GETOPT_INCLUDE_DIR getopt.h)
    include_directories(${GETOPT_INCLUDE_DIR})
    find_library(GETOPT_LIBRARY getopt)
    
    message("-- GETOPT_INCLUDE_DIR: ${GETOPT_INCLUDE_DIR}")
    message("-- GETOPT_LIBRARY: ${GETOPT_LIBRARY}")
else()
    # https://github.com/FXIhub/spsim/blob/master/cmake/Modules/FindMATH.cmake
    find_path(MATH_INCLUDE_DIR math.h)
    include_directories(${MATH_INCLUDE_DIR})

    set(MATH_NAMES ${MATH_NAMES} m)
    find_library(MATH_LIBRARY NAMES ${MATH_NAMES})
endif()

find_package(GTest CONFIG REQUIRED)
find_package(spdlog CONFIG REQUIRED)

# билдим библиотеку Random
set(RANDOM_SHARED_LIBS ON)

if (NOT RANDOM_SHARED_LIBS)
    add_compile_definitions(RANDOM_STATIC_DEFINE)
endif()

add_subdirectory(random)

# Get the current working branch
execute_process(
    COMMAND git rev-parse HEAD
    WORKING_DIRECTORY ${CMAKE_CURRENT_LIST_DIR}
    OUTPUT_VARIABLE GIT_COMMIT_HASH
    OUTPUT_STRIP_TRAILING_WHITESPACE
)
message("-- GIT_COMMIT_HASH: ${GIT_COMMIT_HASH}")

# Get current date
string(TIMESTAMP TODAY "%d%m%Y")

# preprocessing version.h.in to version.h
configure_file(
    ${CMAKE_CURRENT_SOURCE_DIR}/version.h.in 
    ${CMAKE_CURRENT_BINARY_DIR}/version.h
)
include_directories(${PROJECT_BINARY_DIR})

add_custom_target(PrecompiledHeaders SOURCES
    ${CMAKE_CURRENT_SOURCE_DIR}/stable.h
)

# C libraries
set(CL2C
    ${CMAKE_COMMAND} -P "${CMAKE_CURRENT_SOURCE_DIR}/cmake/cl_to_clang.cmake" "${CMAKE_CURRENT_SOURCE_DIR}/2_pi_monte_carlo/kernel_boost.cl")
add_custom_command(
    OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/kernel_boost.cl.c
    COMMAND ${CL2C}
    DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/2_pi_monte_carlo/kernel_boost.cl
    WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
    VERBATIM
)
add_custom_target(cl SOURCES "${CMAKE_CURRENT_SOURCE_DIR}/2_pi_monte_carlo/kernel_boost.cl"
    DEPENDS ${CMAKE_CURRENT_BINARY_DIR}/kernel_boost.cl.c)

add_library(modules STATIC
    ${CMAKE_CURRENT_SOURCE_DIR}/1_weight_combinations/wc_module.h
    ${CMAKE_CURRENT_SOURCE_DIR}/1_weight_combinations/wc_module.c

    ${CMAKE_CURRENT_BINARY_DIR}/kernel_boost.cl.c

    ${CMAKE_CURRENT_SOURCE_DIR}/2_pi_monte_carlo/counters.h
    ${CMAKE_CURRENT_SOURCE_DIR}/2_pi_monte_carlo/counters.c

    ${CMAKE_CURRENT_SOURCE_DIR}/3_polygon_traversal_direction/foo.h
    ${CMAKE_CURRENT_SOURCE_DIR}/3_polygon_traversal_direction/cross_product.c
    ${CMAKE_CURRENT_SOURCE_DIR}/3_polygon_traversal_direction/polygon_traversal_direction.c
)
target_precompile_headers(modules PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/stable.h)
target_link_libraries(modules PRIVATE random::random)

# CPP libraries
add_library(classes STATIC
    ${CMAKE_CURRENT_SOURCE_DIR}/1_weight_combinations/weight_combinator.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/1_weight_combinations/weight_combinator.cpp

    ${CMAKE_CURRENT_SOURCE_DIR}/1_weight_combinations/weight_combinator_io.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/1_weight_combinations/weight_combinator_io.cpp

    ${CMAKE_CURRENT_SOURCE_DIR}/2_pi_monte_carlo/pi_monte_carlo_points_generator_class.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/2_pi_monte_carlo/pi_monte_carlo_points_generator_class.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/2_pi_monte_carlo/true_pi_selector_class.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/2_pi_monte_carlo/true_pi_selector_class.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/2_pi_monte_carlo/pi_convergence_check_class.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/2_pi_monte_carlo/pi_convergence_check_class.cpp

    ${CMAKE_CURRENT_SOURCE_DIR}/3_polygon_traversal_direction/point_class.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/3_polygon_traversal_direction/point_class.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/3_polygon_traversal_direction/cross_product_class.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/3_polygon_traversal_direction/cross_product_class.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/3_polygon_traversal_direction/direction_finder_class.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/3_polygon_traversal_direction/direction_finder_class.cpp
)
target_precompile_headers(classes PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/stable.h)
target_link_libraries(classes
    PRIVATE
        random::random
    PUBLIC
        spdlog::spdlog 
        spdlog::spdlog_header_only
)

# add executables
# Weight Combinations
add_executable(WeightCombinations-C
    ${CMAKE_CURRENT_SOURCE_DIR}/1_weight_combinations/main.c
)
target_link_libraries(WeightCombinations-C PRIVATE
    modules
    # generators
    ${GETOPT_LIBRARY}
    ${MATH_LIBRARY}
)

target_precompile_headers(WeightCombinations-C REUSE_FROM modules)
add_custom_command(TARGET WeightCombinations-C POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
        ${CMAKE_CURRENT_SOURCE_DIR}/1_weight_combinations/source.txt
        $<TARGET_FILE_DIR:WeightCombinations-C>/source_wc.txt
)

add_executable(WeightCombinations-CPP
    ${CMAKE_CURRENT_SOURCE_DIR}/1_weight_combinations/main.cpp
)
target_link_libraries(WeightCombinations-CPP PRIVATE
    classes
    Boost::program_options
    Boost::filesystem
)
target_precompile_headers(WeightCombinations-CPP REUSE_FROM classes)
add_custom_command(TARGET WeightCombinations-CPP POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
        ${CMAKE_CURRENT_SOURCE_DIR}/1_weight_combinations/source.txt
        $<TARGET_FILE_DIR:WeightCombinations-CPP>/source_weight_combinations.txt
)

add_executable(WeightCombinations-TESTS
    ${CMAKE_CURRENT_SOURCE_DIR}/1_weight_combinations/tests.cpp
)
target_link_libraries(WeightCombinations-TESTS PRIVATE
    modules
    classes
    GTest::gmock
    GTest::gtest 
    GTest::gmock_main
    GTest::gtest_main
    Boost::filesystem
)
target_precompile_headers(WeightCombinations-TESTS REUSE_FROM classes)

# # PI Monte Carlo
add_executable(PIMonteCarlo-C 
    ${CMAKE_CURRENT_SOURCE_DIR}/2_pi_monte_carlo/main.c
)
target_link_libraries(PIMonteCarlo-C PRIVATE
    modules
    # generators
    ${GETOPT_LIBRARY}           # empty if build in linux
    ${OpenCL_LIBRARIES}
    ${PThreads4W_LIBRARIES}     # empty if build in linux
)
target_precompile_headers(PIMonteCarlo-C REUSE_FROM modules)
#add_custom_command(TARGET PIMonteCarlo-C POST_BUILD
#    COMMAND ${CMAKE_COMMAND} -E copy_if_different
#        ${CMAKE_CURRENT_SOURCE_DIR}/2_pi_monte_carlo/kernel_boost.cl
#        $<TARGET_FILE_DIR:PIMonteCarlo-C>/kernel_boost.cl
#)
#target_compile_options(PIMonteCarlo-C PRIVATE
#    $<$<OR:$<CXX_COMPILER_ID:Clang>,$<CXX_COMPILER_ID:AppleClang>,$<CXX_COMPILER_ID:GNU>>:
#        -Wall -Wextra -Wpedantic -Werror>
#    $<$<CXX_COMPILER_ID:MSVC>:
#        /W4 /WX>
#)

add_executable(PIMonteCarlo-CPP
   ${CMAKE_CURRENT_SOURCE_DIR}/2_pi_monte_carlo/main.cpp
)
target_link_libraries(PIMonteCarlo-CPP PRIVATE
   classes
   # generators
   Boost::program_options
)
target_precompile_headers(PIMonteCarlo-CPP REUSE_FROM classes)

add_executable(PIMonteCarlo-TESTS
    ${CMAKE_CURRENT_SOURCE_DIR}/2_pi_monte_carlo/tests.cpp
)
target_link_libraries(PIMonteCarlo-TESTS PRIVATE
    modules
    # generators
    ${GETOPT_LIBRARY}           # empty if build in linux
    ${OpenCL_LIBRARIES}
    ${PThreads4W_LIBRARIES}     # empty if build in linux
    classes
    GTest::gmock
    GTest::gtest 
    GTest::gmock_main
    GTest::gtest_main
    Boost::filesystem
)
target_precompile_headers(PIMonteCarlo-TESTS REUSE_FROM classes)
#add_custom_command(TARGET PIMonteCarlo-TESTS POST_BUILD
#    COMMAND ${CMAKE_COMMAND} -E copy_if_different
#        ${CMAKE_CURRENT_SOURCE_DIR}/2_pi_monte_carlo/kernel_boost.cl
#        $<TARGET_FILE_DIR:PIMonteCarlo-TESTS>/kernel_boost.cl
#)

# Polygon Traversal Direction
add_executable(PolygonTraversalDirection-C
    ${CMAKE_CURRENT_SOURCE_DIR}/3_polygon_traversal_direction/main.c
)
target_link_libraries(PolygonTraversalDirection-C PRIVATE
    modules
)
target_precompile_headers(PolygonTraversalDirection-C REUSE_FROM modules)

add_executable(PolygonTraversalDirection-CPP
   ${CMAKE_CURRENT_SOURCE_DIR}/3_polygon_traversal_direction/main.cpp
)
target_link_libraries(PolygonTraversalDirection-CPP PRIVATE
   classes
)
target_precompile_headers(PolygonTraversalDirection-CPP REUSE_FROM classes)
add_custom_command(TARGET PolygonTraversalDirection-CPP POST_BUILD
   COMMAND ${CMAKE_COMMAND} -E copy_if_different
       ${CMAKE_CURRENT_SOURCE_DIR}/3_polygon_traversal_direction/source.txt
       $<TARGET_FILE_DIR:PolygonTraversalDirection-CPP>/source_polygon_traversal_direction.txt
)

add_executable(PolygonTraversalDirection-TESTS
    ${CMAKE_CURRENT_SOURCE_DIR}/3_polygon_traversal_direction/tests.cpp
)
target_link_libraries(PolygonTraversalDirection-TESTS PRIVATE
    modules
    classes
    GTest::gmock
    GTest::gtest 
    GTest::gmock_main
    GTest::gtest_main
    Boost::filesystem
)
target_precompile_headers(PolygonTraversalDirection-TESTS REUSE_FROM classes)
add_custom_command(TARGET PolygonTraversalDirection-TESTS POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
        ${CMAKE_CURRENT_SOURCE_DIR}/3_polygon_traversal_direction/source.txt
        $<TARGET_FILE_DIR:PolygonTraversalDirection-TESTS>/source_polygon_traversal_direction.txt
)

#add_library(time STATIC
#    ${CMAKE_CURRENT_SOURCE_DIR}/time/TimeManagment.hpp
#    ${CMAKE_CURRENT_SOURCE_DIR}/time/TimeManagment.cpp
#)

#add_executable(TimeManagement-TESTS
#    ${CMAKE_CURRENT_SOURCE_DIR}/time/tests.cpp
#)
#target_link_libraries(TimeManagement-TESTS PRIVATE
#    classes
#    GTest::gmock
#    GTest::gtest
#    GTest::gmock_main
#    GTest::gtest_main
#    Boost::filesystem
#    time
#)
#target_precompile_headers(TimeManagement-TESTS REUSE_FROM classes)

# Doxygen generate docs
# https://vicrucann.github.io/tutorials/quick-cmake-doxygen/
option(BUILD_DOC "Build documentation" ON)

# graphviz https://community.chocolatey.org/packages/Graphviz
# https://github.com/labapart/gattlib/issues/129

if (BUILD_DOC)
    find_package(Doxygen 1.9.5)
    if (DOXYGEN_FOUND)
        # set input and output files
        set(DOXYGEN_IN ${CMAKE_CURRENT_SOURCE_DIR}/Doxyfile.in)
        set(DOXYGEN_OUT ${CMAKE_CURRENT_BINARY_DIR}/Doxyfile)

        # request to configure the file
        configure_file(${DOXYGEN_IN} ${DOXYGEN_OUT} @ONLY)
        message("-- Doxygen build started")

        # note the option ALL which allows to build the docs together with the application
        add_custom_target( doc_doxygen ALL
            COMMAND ${DOXYGEN_EXECUTABLE} ${DOXYGEN_OUT}
            WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
            COMMENT "-- Generating API documentation with Doxygen"
            VERBATIM
        )
    else (DOXYGEN_FOUND)
        message("-- Doxygen need to be installed to generate the doxygen documentation")
    endif (DOXYGEN_FOUND)
endif (BUILD_DOC)

add_subdirectory(generator_research)
