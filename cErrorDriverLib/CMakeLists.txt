# ##############################################################################
# CMakeLists.txt — cErrorDriver
#
# Builds the error driver library (cErrorDriver) as either a static or shared
# library depending on the COMPILE_ERROR_DRIVER_LIBRARY_STATIC flag set by the
# parent CMakeLists.txt.
#
# Library layout:
#   source/       — private implementation (.c files compiled into the library)
#   include/      — private headers (only visible inside this library)
#   publicInclude/— public API headers (propagated to all consumers via PUBLIC)
#
# Dependencies:
#   commonMacros  — header-only INTERFACE target providing shared project macros
#   commonTypes   — header-only INTERFACE target providing shared project types.
# Version header:
#   cErrorDriverVersion.h is auto-generated into the build tree on every
#   cmake --build invocation via generateVersion.cmake. It is written to:
#     <publicInclude>/generated/cErrorDriverVersion.h
#   and exposes ERROR_DRIVER_VERSION_MAJOR/MINOR/PATCH/BUILD_TYPE/STRING.
#   A stub is written at configure time so the file exists for dep scanning.
#
# Unit tests:
#   Conditionally compiled when UNIT_TESTS is ON and the test source file
#   unittest/cErrorDriver_test.cpp exists. Linked against GTest::gtest_main
#   and discovered automatically via gtest_discover_tests().
# ##############################################################################

# --- Version header setup -----------------------------------------------------
set(VERSION_HEADER_DIR  ${CMAKE_CURRENT_SOURCE_DIR}/publicInclude/generated)
set(VERSION_HEADER_FILE ${VERSION_HEADER_DIR}/cErrorDriverVersion.h)

if( COMPILE_ERROR_DRIVER_LIBRARY_STATIC )
    set(ERROR_DRIVER_BUILD_TYPE "STATIC")
    set(ERROR_DRIVER_BUILD_TYPE_ENUM STATIC_LIBRARY_BUILD)
else()
    set(ERROR_DRIVER_BUILD_TYPE "DYNAMIC")
    set(ERROR_DRIVER_BUILD_TYPE_ENUM DYNAMIC_LIBRARY_BUILD)
endif()

# Write a stub at configure time so the path exists and dependency scanning works
# before the real file is generated on the first build.
file(MAKE_DIRECTORY ${VERSION_HEADER_DIR})
if(NOT EXISTS ${VERSION_HEADER_FILE})
    file(WRITE ${VERSION_HEADER_FILE}
        "/* Stub — replaced at build time by generateVersion.cmake */\n"
        "#ifndef C_ERROR_DRIVER_VERSION_H\n"
        "#define C_ERROR_DRIVER_VERSION_H\n"
        "#endif /* C_ERROR_DRIVER_VERSION_H */\n"
    )
endif()
# ------------------------------------------------------------------------------


if( COMPILE_ERROR_DRIVER_LIBRARY_STATIC )
    add_library(${ERROR_DRIVER_LIBRARY_NAME} STATIC
        source/cErrorDriver.c
        source/cCommonErrors.c
    )
else()
    add_library(${ERROR_DRIVER_LIBRARY_NAME} SHARED
        source/cErrorDriver.c
        source/cCommonErrors.c
    )
endif()

target_include_directories(${ERROR_DRIVER_LIBRARY_NAME}
    PRIVATE
        include
        ../../cCommonTypes/include
        ../../cLoggingDriver/cLoggingDriverLib/publicInclude
        ../../cSemaphore/cSemaphoreLib/publicInclude
    PUBLIC
        publicInclude
)

target_link_libraries(${ERROR_DRIVER_LIBRARY_NAME}
    PUBLIC
        cCommonMacros
        commonTypes
        cLoggingDriverLib
        cSemaphoreLib
)

# Expose the generated header directory so consumers can #include it
target_include_directories(${ERROR_DRIVER_LIBRARY_NAME}
    PUBLIC
        ${VERSION_HEADER_DIR}
)

# Always regenerate cErrorDriverVersion.h at every build invocation.
# add_custom_target runs unconditionally (no output tracking), ensuring a
# fresh timestamp in PATCH on every cmake --build call.
add_custom_target(GenerateErrorDriverVersion ALL
    COMMAND ${CMAKE_COMMAND}
        -DVERSION_MAJOR=${PROJECT_VERSION_MAJOR}
        -DVERSION_MINOR=${PROJECT_VERSION_MINOR}
        -DVERSION_PATCH=${PROJECT_VERSION_PATCH}
        -DVERSION_BUILD_TYPE=${ERROR_DRIVER_BUILD_TYPE}
        -DVERSION_BUILD_TYPE_ENUM=${ERROR_DRIVER_BUILD_TYPE_ENUM}
        -DOUTPUT_FILE=${VERSION_HEADER_FILE}
        -P ${CMAKE_CURRENT_SOURCE_DIR}/generateVersion.cmake
    COMMENT "Regenerating cErrorDriverVersion.h"
    VERBATIM
)

# Ensure the header exists before the library sources are compiled
add_dependencies(${ERROR_DRIVER_LIBRARY_NAME} GenerateErrorDriverVersion)


if( UNIT_TESTS AND EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/unittest/cErrorDriver_test.cpp )
    add_executable(
        cErrorDriverTest
        unittest/cErrorDriver_test.cpp
    )

    target_include_directories(cErrorDriverTest
        PRIVATE
            include
        ../../cCommonMacros/include
        ../../cCommonTypes/include
    )

    target_link_libraries(
        cErrorDriverTest
        ${ERROR_DRIVER_LIBRARY_NAME}
        GTest::gtest_main
    )

    include(GoogleTest)
    gtest_discover_tests(cErrorDriverTest)
endif()
