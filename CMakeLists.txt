cmake_minimum_required(VERSION 3.1...3.22)
project(tws VERSION 0.0.1 LANGUAGES C)
set(CMAKE_MODULE_PATH ${tws_SOURCE_DIR}/cmake ${CMAKE_MODULE_PATH})

set(CMAKE_CXX_STANDARD_REQUIRED True)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

if(CMAKE_VERSION VERSION_LESS 3.0.0)
    set(CMAKE_INSTALL_LIBDIR "lib" CACHE PATH "library install dir (lib)")
    set(CMAKE_INSTALL_INCLUDEDIR "include" CACHE PATH "header base install dir (include)")
    set(CMAKE_INSTALL_BINDIR "bin" CACHE PATH "user executables (bin)")
else()
    cmake_policy(SET CMP0042 NEW)
    cmake_policy(SET CMP0046 NEW)
    include(GNUInstallDirs)
endif()

if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE "Debug" CACHE STRING
      "Choose the type of build, options are: Debug Release" FORCE)
endif()

set(TWS_VERSION_MAJOR 0)
set(TWS_VERSION_MINOR 1)
set(TWS_VERSION_PATCH 0)
set(TWS_VERSION "${TWS_VERSION_MAJOR}.${TWS_VERSION_MINOR}.${TWS_VERSION_PATCH}")
message("Configuring TWS version ${TWS_VERSION}")

if(NOT TWS_STATIC_LIBNAME)
    set(TWS_STATIC_LIBNAME tws)
endif()

option(BUILD_STATIC_LIBS "Build static libraries" ON)
option(BUILD_DYNAMIC_LIBS "Build dynamic libraries" ON)
option(BUILD_EXAMPLE_PROGRAMS "Build example programs" ON)
option(BUILD_TESTS "Build tests" ON)

set(TWS_SOURCES
    "${tws_SOURCE_DIR}/src/tws.c"
)

set(TWS_HEADERS
    "${tws_SOURCE_DIR}/tws/tws.h"
)

include_directories(
    ${tws_SOURCE_DIR}
    ${tws_BINARY_DIR}
    "${tws_SOURCE_DIR}/test"
    "${tws_SOURCE_DIR}/src"
)

if(MSVC)
    set(CMAKE_C_FLAGS_DEBUG "${CMAKE_C_FLAGS_DEBUG} /Wall")
    set(LIB_CFLAGS "/TP /W4")
    set(EXAMPLE_CFLAGS "/W4")
    set(TEST_CFLAGS "${LIB_CFLAGS}")
    set(TEST_LDFLAGS " ")
    set(LIBM " ")
else()
    set(CMAKE_C_FLAGS_DEBUG "${CMAKE_C_FLAGS_DEBUG} -Werror -pedantic")
    set(LIB_CFLAGS "-std=c11 -fvisibility=hidden -Wall -Werror=strict-prototypes -Werror=old-style-definition -Werror=missing-prototypes -D_REENTRANT -D_POSIX_C_SOURCE=200809L -Wno-missing-braces")
    set(EXAMPLE_CFLAGS "-std=c99 -Wall")
    set(TEST_CFLAGS "${LIB_CFLAGS} -fprofile-arcs -ftest-coverage")
    set(TEST_LDFLAGS "-fprofile-arcs -ftest-coverage")
    set(LIBM "m")
endif()

if(BUILD_DYNAMIC_LIBS)
    add_library(tws_shared SHARED ${TWS_SOURCES})
    set_target_properties(tws_shared PROPERTIES
        OUTPUT_NAME tws
        SOVERSION ${TWS_VERSION_MAJOR}
        VERSION ${TWS_VERSION}
        COMPILE_FLAGS ${LIB_CFLAGS}
        LINKER_LANGUAGE C
    )
    install(TARGETS tws_shared DESTINATION ${CMAKE_INSTALL_LIBDIR})
endif()

if(BUILD_STATIC_LIBS)
    add_library(tws_static STATIC ${TWS_SOURCES})
    set_target_properties(tws_static PROPERTIES
        OUTPUT_NAME ${TWS_STATIC_LIBNAME}
        COMPILE_FLAGS ${LIB_CFLAGS}
        LINKER_LANGUAGE C
    )
    install(TARGETS tws_static DESTINATION ${CMAKE_INSTALL_LIBDIR})
endif()

install(FILES
    ${TWS_HEADERS}
    DESTINATION "${CMAKE_INSTALL_INCLUDEDIR}/tws")

# Example Programs
if(BUILD_EXAMPLE_PROGRAMS)
    add_executable(example examples/example.c)
    set_target_properties(example PROPERTIES
        LINKER_LANGUAGE C
        COMPILE_FLAGS ${EXAMPLE_CFLAGS})
        target_link_libraries(example tws_shared ${LIBM})
    install(TARGETS example DESTINATION ${CMAKE_INSTALL_BINDIR})
endif()

if(BUILD_TESTS)
    # TODO
endif()

message("\n"
    "Installation Summary\n"
    "--------------------\n"
    "* Install Directory            : ${CMAKE_INSTALL_PREFIX}\n"
    "* Build Type                   : ${CMAKE_BUILD_TYPE}\n"
    "* Build static libs            : ${BUILD_STATIC_LIBS}\n"
    "* Build examples               : ${BUILD_EXAMPLE_PROGRAMS}\n"
    "* Build tests                  : ${BUILD_TESTS}\n"
)

