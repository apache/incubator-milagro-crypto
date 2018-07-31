# Licensed to the Apache Software Foundation (ASF) under one
# or more contributor license agreements.  See the NOTICE file
# distributed with this work for additional information
# regarding copyright ownership.  The ASF licenses this file
# to you under the Apache License, Version 2.0 (the
# "License"); you may not use this file except in compliance
# with the License.  You may obtain a copy of the License at
#
#      http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing,
# software distributed under the License is distributed on an
# "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
# KIND, either express or implied.  See the License for the
# specific language governing permissions and limitations
# under the License.

cmake_minimum_required(VERSION 3.1 FATAL_ERROR)

file(STRINGS "VERSION" VERSION_STRING)
project(AMCL VERSION "${VERSION_STRING}" LANGUAGES C)

##################################################
# Helper Macros
##################################################
macro(log var)
  message(STATUS "${var}: ${${var}}")
endmacro()

macro(set_if_unset var val)
  if(NOT ${var})
    set(${var} "${val}")
  endif()
  log(${var})
endmacro()

##################################################
# Includes
##################################################
list(APPEND CMAKE_MODULE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/cmake)
include(AMCLExpand)
include(CMakeDependentOption)
include(CTest)
include(DetermineWordSize)
include(GNUInstallDirs)
include(SetOSVariable)

##################################################
# Internal settings
##################################################
message(STATUS "AMCL")
log(PROJECT_VERSION)
log(CMAKE_GENERATOR)

add_definitions(-DCMAKE)

set(AMCL_VERSION       ${PROJECT_VERSION})
set(AMCL_VERSION_MAJOR ${PROJECT_VERSION_MAJOR})
set(AMCL_VERSION_MINOR ${PROJECT_VERSION_MINOR})
set(AMCL_VERSION_PATCH ${PROJECT_VERSION_PATCH})

set(AMCL_SOVERSION ${AMCL_VERSION_MAJOR})

if(CMAKE_COMPILER_IS_GNUCC)
    execute_process(COMMAND ${CMAKE_C_COMPILER} -dumpversion
                    OUTPUT_VARIABLE GCC_VERSION)
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -std=c99 -static-libgcc -Wall -Wextra -Wno-strict-prototypes -Wunused-value -Wcast-align -Wunused-variable -Wundef -Wformat-security")

    if (GCC_VERSION VERSION_GREATER 4.8 OR GCC_VERSION VERSION_EQUAL 4.8)
        set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wshadow")
        set(CMAKE_C_FLAGS_ASAN    "-O0 -g3 -fsanitize=address")
    else (GCC_VERSION VERSION_GREATER 4.8 OR GCC_VERSION VERSION_EQUAL 4.8)
        message(STATUS "GCC 4.8 required to run address sanitizer - please upgrade your installation")
    endif(GCC_VERSION VERSION_GREATER 4.8 OR GCC_VERSION VERSION_EQUAL 4.8)

    set(CMAKE_C_FLAGS_RELEASE     "-O2")
    set(CMAKE_C_FLAGS_DEBUG       "-O0 -g3")
    set(CMAKE_C_FLAGS_COVERAGE    "-O0 -g3 --coverage")
    set(CMAKE_C_FLAGS_CHECK       "-O2 -Werror")
    set(CMAKE_C_FLAGS_CHECKFULL   "${CMAKE_C_FLAGS_CHECK} -Wcast-qual")
endif(CMAKE_COMPILER_IS_GNUCC)

if(CMAKE_BUILD_TYPE STREQUAL "Coverage")
    set(CMAKE_SHARED_LINKER_FLAGS "--coverage")
endif(CMAKE_BUILD_TYPE STREQUAL "Coverage")

##################################################
# Options
##################################################
### Build Type ###
if(NOT CMAKE_BUILD_TYPE AND NOT CMAKE_CONFIGURATION_TYPES)
  set(CMAKE_BUILD_TYPE "Release" CACHE STRING "Choose type of build." FORCE)
endif()
set_property(CACHE CMAKE_BUILD_TYPE PROPERTY STRINGS "Release" "Debug" "Coverage" "ASan" "Check" "CheckFull")
log(CMAKE_BUILD_TYPE)

### Configurations ###
set_if_unset(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/bin)
set_if_unset(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/lib)
set_if_unset(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/lib)
log(CMAKE_INSTALL_PREFIX)

### Options ###
option(BUILD_SHARED_LIBS "Build shared libraries" ON)
log(BUILD_SHARED_LIBS)

option(BUILD_BENCHMARKS  "Build benchmarks"       ON)
option(BUILD_DOCS        "Build docs"             ON)
option(BUILD_EXAMPLES    "Build examples"         ON)
cmake_dependent_option(BUILD_PYTHON "Build Python" OFF "BUILD_SHARED_LIBS" OFF)
log(BUILD_BENCHMARKS)
log(BUILD_DOCS)
log(BUILD_EXAMPLES)
log(BUILD_PYTHON)
log(BUILD_TESTING) # added by 'include(CTest)'

option(BUILD_MPIN    "Build MPIN"    ON)
option(BUILD_WCC     "Build WCC"     ON)
option(BUILD_X509    "BUild X509"    ON)
log(BUILD_MPIN)
log(BUILD_WCC)
log(BUILD_X509)

option(DEBUG_REDUCE "Print debug message for field reduction" OFF)
option(DEBUG_NORM "Detect digit overflow" OFF)
option(GET_STATS "Debug statistics" OFF)
log(DEBUG_REDUCE)
log(DEBUG_NORM)
log(GET_STATS)

### /include subdir ###
set(AMCL_INCLUDE_SUBDIR "" CACHE STRING "Subdir of 'include' to install headers in")
set(INSTALL_INCLUDESUBDIR "${CMAKE_INSTALL_INCLUDEDIR}/${AMCL_INCLUDE_SUBDIR}")
log(AMCL_INCLUDE_SUBDIR)
log(CMAKE_INSTALL_INCLUDEDIR)
log(INSTALL_INCLUDESUBDIR)

### Word/Chunk Size ###
determine_word_size(DEFAULT_WORD_SIZE)
set(WORD_SIZE ${DEFAULT_WORD_SIZE} CACHE STRING "Word length in bits. See ./include/arch.h")
set_property(CACHE WORD_SIZE PROPERTY STRINGS "16;32;64")
log(WORD_SIZE)

if(WORD_SIZE STREQUAL "")
  message(FATAL_ERROR "Must explicitly set WORD_SIZE.")
endif()

### RSA Levels ###
amcl_supported_rsa_levels(AMCL_RSA_LEVELS ${WORD_SIZE})
set(AMCL_RSA "${AMCL_RSA_LEVELS}" CACHE STRING "RSA levels of security supported. See ./include/rsa_WWW.h")
set_property(CACHE AMCL_RSA PROPERTY STRINGS ${AMCL_RSA_LEVELS})
string(REPLACE "," ";" AMCL_RSA "${AMCL_RSA}")
log(AMCL_RSA)

### Curves ###
amcl_supported_curves(AMCL_CURVE_NAMES ${WORD_SIZE})
set(AMCL_CURVE "${AMCL_CURVE_NAMES}" CACHE STRING "Choice of Field. See ./include/amcl.h")
set_property(CACHE AMCL_CURVE PROPERTY STRINGS ${AMCL_CURVE_NAMES})
string(REPLACE "," ";" AMCL_CURVE "${AMCL_CURVE}")
log(AMCL_CURVE)

### MPIN ###
if(BUILD_MPIN)
  set(AMCL_MAXPIN 10000 CACHE STRING "Maximum PIN value. See ./include/mpin_ZZZ.h for explanation.")
  set(AMCL_PBLEN 14 CACHE STRING "Maximum PIN value in bits. See ./include/mpin_ZZZ.h for explanation.")
  log(AMCL_MAXPIN)
  log(AMCL_PBLEN)
endif()

### PBC ###
if(BUILD_MPIN OR BUILD_WCC)
  set(PBC_TIME_PERMIT_TESTS 10 CACHE STRING "Number of days in the future to test M-PIN/WCC time permits")
  set(PBC_RANDOM_TESTS 10 CACHE STRING "Number of random M-PIN/WCC tests")
  log(PBC_TIME_PERMIT_TESTS)
  log(PBC_RANDOM_TESTS)
endif()

##################################################
# AMCL_Core Library
##################################################
amcl_configure_file_core(../c/amcl.h include/amcl.h amcl_core_GEN_HDRS)
amcl_configure_file_core(../c/arch.h include/arch.h amcl_core_GEN_HDRS)

amcl_generate_SC(SC)
amcl_generate_RSL(RSL)
amcl_configure_file_core(include/version.h.in include/version.h amcl_core_GEN_HDRS)
unset(SC)
unset(RSL)

configure_file(../c/hash.c src/hash.c COPYONLY)
configure_file(../c/rand.c src/rand.c COPYONLY)
configure_file(../c/randapi.h include/randapi.h COPYONLY)
configure_file(../c/randapi.c src/randapi.c COPYONLY)
configure_file(../c/aes.c src/aes.c COPYONLY)
configure_file(../c/gcm.c src/gcm.c COPYONLY)
configure_file(../c/oct.c src/oct.c COPYONLY)
add_library(amcl_core
  src/hash.c
  src/rand.c
  src/randapi.c
  src/aes.c
  src/gcm.c
  src/oct.c
  src/utils.c
  src/version.c
)
list(APPEND AMCL_LIBRARIES amcl_core)

set_target_properties(amcl_core PROPERTIES
  EXPORT_NAME core
  VERSION ${AMCL_VERSION}
  SOVERSION ${AMCL_SOVERSION}
)

target_include_directories(amcl_core PUBLIC
  $<BUILD_INTERFACE:${CMAKE_CURRENT_BINARY_DIR}/include>
  $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
  $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>
)

install(TARGETS amcl_core
  EXPORT AMCLTargets
  RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
  LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
  ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
)

install(FILES
  ${amcl_core_GEN_HDRS}
  include/utils.h
  target/default/include/randapi.h
  DESTINATION ${INSTALL_INCLUDESUBDIR}
)

if (BUILD_MPIN OR BUILD_WCC)
  amcl_configure_file_core(include/config_test.h.in include/config_test.h amcl_core_pbc_GEN_HDRS)

  configure_file(../c/pbc_support.h include/pbc_support.h COPYONLY)
  configure_file(../c/pbc_support.c src/pbc_support.c COPYONLY)
  target_sources(amcl_core PRIVATE src/pbc_support.c)

  install(FILES
    ${amcl_core_pbc_GEN_HDRS}
    target/default/include/pbc_support.h
    DESTINATION ${INSTALL_INCLUDESUBDIR}
  )
endif()

if (NOT AMCL_RSA STREQUAL "")
  configure_file(../c/rsa_support.h include/rsa_support.h COPYONLY)
  configure_file(../c/rsa_support.c src/rsa_support.c COPYONLY)
  target_sources(amcl_core PRIVATE src/rsa_support.c)

  install(FILES
    target/default/include/rsa_support.h
    DESTINATION ${INSTALL_INCLUDESUBDIR}
  )
endif()

if (NOT AMCL_CURVE STREQUAL "")
  configure_file(../c/ecdh_support.h include/ecdh_support.h COPYONLY)
  configure_file(../c/ecdh_support.c src/ecdh_support.c COPYONLY)
  target_sources(amcl_core PRIVATE src/ecdh_support.c)

  install(FILES
    target/default/include/ecdh_support.h
    DESTINATION ${INSTALL_INCLUDESUBDIR}
  )
endif()

##################################################
# AMCL_X509 Library
##################################################
if(BUILD_X509)
  message(STATUS "Build libamcl_x509")
  configure_file(../c/x509.h include/x509.h COPYONLY)
  configure_file(../c/x509.c src/x509.c COPYONLY)
  add_library(amcl_x509
    src/x509.c
  )
  list(APPEND AMCL_LIBRARIES amcl_x509)

  set_target_properties(amcl_x509 PROPERTIES
    EXPORT_NAME x509
    VERSION ${AMCL_VERSION}
    SOVERSION ${AMCL_SOVERSION}
  )

  target_include_directories(amcl_x509 PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_BINARY_DIR}/include>
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>
  )

  target_link_libraries(amcl_x509 PUBLIC
    amcl_core
  )

  install(TARGETS amcl_x509
    EXPORT AMCLTargets
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
  )

  install(FILES
    target/default/include/x509.h
    DESTINATION ${INSTALL_INCLUDESUBDIR}
  )
endif()

##################################################
# AMCL_RSA_*** Libraries
##################################################
foreach(level ${AMCL_RSA})
  amcl_rsa_field(BD "${level}")
  amcl_rsa_field(TFF "${level}")

  amcl_configure_file_rsa(../c/config_big.h include/config_big_${BD}.h "${level}" amcl_rsa_${level}_GEN_HDRS)
  amcl_configure_file_rsa(../c/config_ff.h  include/config_ff_${TFF}.h "${level}" amcl_rsa_${level}_GEN_HDRS)
  amcl_configure_file_rsa(../c/big.h        include/big_${BD}.h        "${level}" amcl_rsa_${level}_GEN_HDRS)
  amcl_configure_file_rsa(../c/ff.h         include/ff_${TFF}.h        "${level}" amcl_rsa_${level}_GEN_HDRS)
  amcl_configure_file_rsa(../c/rsa.h        include/rsa_${TFF}.h       "${level}" amcl_rsa_${level}_GEN_HDRS)

  amcl_configure_file_rsa(../c/big.c src/big_${BD}.c  "${level}" amcl_rsa_${level}_GEN_SRCS)
  amcl_configure_file_rsa(../c/ff.c  src/ff_${TFF}.c  "${level}" amcl_rsa_${level}_GEN_SRCS)
  amcl_configure_file_rsa(../c/rsa.c src/rsa_${TFF}.c "${level}" amcl_rsa_${level}_GEN_SRCS)

  message(STATUS "Build libamcl_rsa_${TFF}")
  add_library(amcl_rsa_${level}
    ${amcl_rsa_${level}_GEN_SRCS}
  )
  list(APPEND AMCL_LIBRARIES amcl_rsa_${TFF})

  set_target_properties(amcl_rsa_${level} PROPERTIES
    EXPORT_NAME rsa_${level}
    VERSION ${AMCL_VERSION}
    SOVERSION ${AMCL_SOVERSION}
  )

  target_include_directories(amcl_rsa_${level} PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_BINARY_DIR}/include>
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>
  )

  target_link_libraries(amcl_rsa_${level} PUBLIC
    amcl_core
  )

  install(TARGETS amcl_rsa_${level}
    EXPORT AMCLTargets
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
  )

  install(FILES
    ${amcl_rsa_${level}_GEN_HDRS}
    DESTINATION ${INSTALL_INCLUDESUBDIR}
  )

endforeach()

##################################################
# AMCL_CURVE_*** Libraries
##################################################
foreach(curve ${AMCL_CURVE})
  amcl_curve_field(BD "${curve}")
  amcl_curve_field(PF "${curve}")
  amcl_curve_field(TC "${curve}")
  amcl_curve_field(TF "${curve}")

  amcl_configure_file_curve(../c/big.h          include/big_${BD}.h          "${curve}" amcl_curve_${TC}_GEN_HDRS)
  amcl_configure_file_curve(../c/config_big.h   include/config_big_${BD}.h   "${curve}" amcl_curve_${TC}_GEN_HDRS)
  amcl_configure_file_curve(../c/config_field.h include/config_field_${TF}.h "${curve}" amcl_curve_${TC}_GEN_HDRS)
  amcl_configure_file_curve(../c/config_curve.h include/config_curve_${TC}.h "${curve}" amcl_curve_${TC}_GEN_HDRS)
  amcl_configure_file_curve(../c/fp.h           include/fp_${TF}.h           "${curve}" amcl_curve_${TC}_GEN_HDRS)
  amcl_configure_file_curve(../c/ecdh.h         include/ecdh_${TC}.h         "${curve}" amcl_curve_${TC}_GEN_HDRS)
  amcl_configure_file_curve(../c/ecp.h          include/ecp_${TC}.h          "${curve}" amcl_curve_${TC}_GEN_HDRS)

  amcl_configure_file_curve(../c/big.c  src/big_${BD}.c  "${curve}" amcl_curve_${TC}_GEN_SRCS)
  amcl_configure_file_curve(../c/fp.c   src/fp_${TF}.c   "${curve}" amcl_curve_${TC}_GEN_SRCS)
  amcl_configure_file_curve(../c/ecp.c  src/ecp_${TC}.c  "${curve}" amcl_curve_${TC}_GEN_SRCS)
  amcl_configure_file_curve(../c/ecdh.c src/ecdh_${TC}.c "${curve}" amcl_curve_${TC}_GEN_SRCS)

  message(STATUS "Build libamcl_curve_${TC}")
  configure_file(../c/rom_curve_${TC}.c src/rom_curve_${TC}.c COPYONLY)
  configure_file(../c/rom_field_${TF}.c src/rom_field_${TF}.c COPYONLY)
  add_library(amcl_curve_${TC}
    ${amcl_curve_${TC}_GEN_SRCS}
    src/rom_curve_${TC}.c
    src/rom_field_${TF}.c
  )
  list(APPEND AMCL_LIBRARIES amcl_curve_${TC})

  set_target_properties(amcl_curve_${TC} PROPERTIES
    EXPORT_NAME curve_${TC}
    VERSION ${AMCL_VERSION}
    SOVERSION ${AMCL_SOVERSION}
  )

  target_include_directories(amcl_curve_${TC} PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_BINARY_DIR}/include>
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>
  )

  target_link_libraries(amcl_curve_${TC} PUBLIC
    amcl_core
  )

  install(TARGETS amcl_curve_${TC}
    EXPORT AMCLTargets
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
  )

  install(FILES
    ${amcl_curve_${TC}_GEN_HDRS}
    DESTINATION ${INSTALL_INCLUDESUBDIR}
  )
endforeach()

##################################################
# AMCL_PAIRING_*** Libraries
##################################################
foreach(curve ${AMCL_CURVE})
  amcl_curve_field(PF "${curve}")
  amcl_curve_field(TC "${curve}")
  amcl_curve_field(TF "${curve}")

  if(NOT(PF STREQUAL "NOT"))
    amcl_configure_file_curve(../c/fp2.h  include/fp2_${TF}.h  "${curve}" amcl_pairing_${TC}_GEN_HDRS)
    amcl_configure_file_curve(../c/fp4.h  include/fp4_${TF}.h  "${curve}" amcl_pairing_${TC}_GEN_HDRS)
    amcl_configure_file_curve(../c/fp12.h include/fp12_${TF}.h "${curve}" amcl_pairing_${TC}_GEN_HDRS)
    amcl_configure_file_curve(../c/ecp2.h include/ecp2_${TC}.h "${curve}" amcl_pairing_${TC}_GEN_HDRS)
    amcl_configure_file_curve(../c/pair.h include/pair_${TC}.h "${curve}" amcl_pairing_${TC}_GEN_HDRS)

    amcl_configure_file_curve(../c/fp2.c  src/fp2_${TC}.c  "${curve}" amcl_pairing_${TC}_GEN_SRCS)
    amcl_configure_file_curve(../c/fp4.c  src/fp4_${TC}.c  "${curve}" amcl_pairing_${TC}_GEN_SRCS)
    amcl_configure_file_curve(../c/fp12.c src/fp12_${TC}.c "${curve}" amcl_pairing_${TC}_GEN_SRCS)
    amcl_configure_file_curve(../c/ecp2.c src/ecp2_${TC}.c "${curve}" amcl_pairing_${TC}_GEN_SRCS)
    amcl_configure_file_curve(../c/pair.c src/pair_${TC}.c "${curve}" amcl_pairing_${TC}_GEN_SRCS)

    message(STATUS "Build libamcl_pairing_${TC}")
    add_library(amcl_pairing_${TC}
      ${amcl_pairing_${TC}_GEN_SRCS}
    )
    list(APPEND AMCL_LIBRARIES amcl_pairing_${TC})

    set_target_properties(amcl_pairing_${TC} PROPERTIES
      EXPORT_NAME pairing_${TC}
      VERSION ${AMCL_VERSION}
      SOVERSION ${AMCL_SOVERSION}
    )

    target_include_directories(amcl_pairing_${TC} PUBLIC
      $<BUILD_INTERFACE:${CMAKE_CURRENT_BINARY_DIR}/include>
      $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
      $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>
    )

    target_link_libraries(amcl_pairing_${TC} PUBLIC
      amcl_curve_${TC}
    )

    install(TARGETS amcl_pairing_${TC}
      EXPORT AMCLTargets
      RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
      LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
      ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
    )

    install(FILES
      ${amcl_pairing_${TC}_GEN_HDRS}
      DESTINATION ${INSTALL_INCLUDESUBDIR}
    )
  endif()
endforeach()

##################################################
# AMCL_MPIN_*** Libraries
##################################################
if(BUILD_MPIN)
  foreach(curve ${AMCL_CURVE})
    amcl_curve_field(TC "${curve}")

    if(TARGET amcl_pairing_${TC})
      amcl_configure_file_curve(../c/mpin.h include/mpin_${TC}.h "${curve}" amcl_mpin_${TC}_GEN_HDRS)
      amcl_configure_file_curve(../c/mpin.c     src/mpin_${TC}.c     "${curve}" amcl_mpin_${TC}_GEN_SRCS)

      message(STATUS "Build libamcl_mpin_${TC}")
      add_library(amcl_mpin_${TC}
        ${amcl_mpin_${TC}_GEN_SRCS}
      )
      list(APPEND AMCL_LIBRARIES amcl_mpin_${TC})

      set_target_properties(amcl_mpin_${TC} PROPERTIES
        EXPORT_NAME mpin_${TC}
        VERSION ${AMCL_VERSION}
        SOVERSION ${AMCL_SOVERSION}
      )

      target_include_directories(amcl_mpin_${TC} PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_BINARY_DIR}/include>
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
        $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>
      )

      target_link_libraries(amcl_mpin_${TC} PUBLIC
        amcl_pairing_${TC}
      )

      install(TARGETS amcl_mpin_${TC}
        EXPORT AMCLTargets
        RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
        LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
        ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
      )

      install(FILES
        ${amcl_mpin_${TC}_GEN_HDRS}
        DESTINATION ${INSTALL_INCLUDESUBDIR}
      )
    endif()
  endforeach()
endif()

##################################################
# AMCL_WCC_*** Libraries
##################################################
if(BUILD_WCC)
  foreach(curve ${AMCL_CURVE})
    amcl_curve_field(TC "${curve}")

    if(TARGET amcl_pairing_${TC})
      amcl_configure_file_curve(include/wcc.h.in include/wcc_${TC}.h "${curve}" amcl_wcc_${TC}_GEN_HDRS)
      amcl_configure_file_curve(src/wcc.c.in     src/wcc_${TC}.c     "${curve}" amcl_wcc_${TC}_GEN_SRCS)

      message(STATUS "Build libamcl_wcc_${TC}")
      add_library(amcl_wcc_${TC}
        ${CMAKE_CURRENT_BINARY_DIR}/src/wcc_${TC}.c
      )
      list(APPEND AMCL_LIBRARIES amcl_wcc_${TC})

      set_target_properties(amcl_wcc_${TC} PROPERTIES
        EXPORT_NAME wcc_${TC}
        VERSION ${AMCL_VERSION}
        SOVERSION ${AMCL_SOVERSION}
      )

      target_include_directories(amcl_wcc_${TC} PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_BINARY_DIR}/include>
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
        $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>
      )

      target_link_libraries(amcl_wcc_${TC} PUBLIC
        amcl_pairing_${TC}
      )

      install(TARGETS amcl_wcc_${TC}
        EXPORT AMCLTargets
        RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
        LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
        ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
      )

      install(FILES
        ${amcl_wcc_${TC}_GEN_HDRS}
        DESTINATION ${INSTALL_INCLUDESUBDIR}
      )
    endif()
  endforeach()
endif()

##################################################
# pkgconfig
##################################################
foreach(lib ${AMCL_LIBRARIES})
  set(AMCL_PRIVATE_LIBS "${AMCL_PRIVATE_LIBS} -l${lib}")
endforeach()
configure_file(amcl.pc.in amcl.pc @ONLY)
install(FILES
  ${CMAKE_CURRENT_BINARY_DIR}/amcl.pc
  DESTINATION ${CMAKE_INSTALL_LIBDIR}/pkgconfig
)

##################################################
# CMake export
##################################################
set(INSTALL_CONFIGDIR ${CMAKE_INSTALL_LIBDIR}/cmake/amcl)

install(EXPORT AMCLTargets
  FILE AMCLTargets.cmake
  NAMESPACE AMCL::
  DESTINATION ${INSTALL_CONFIGDIR}
)

include(CMakePackageConfigHelpers)
write_basic_package_version_file(
  ${CMAKE_CURRENT_BINARY_DIR}/AMCLConfigVersion.cmake
  VERSION ${AMCL_VERSION}
  COMPATIBILITY SameMajorVersion
)

configure_package_config_file(${CMAKE_CURRENT_SOURCE_DIR}/AMCLConfig.cmake.in
  ${CMAKE_CURRENT_BINARY_DIR}/AMCLConfig.cmake
  INSTALL_DESTINATION ${INSTALL_CONFIGDIR}
)

install(FILES
  ${CMAKE_CURRENT_BINARY_DIR}/AMCLConfig.cmake
  ${CMAKE_CURRENT_BINARY_DIR}/AMCLConfigVersion.cmake
  DESTINATION ${INSTALL_CONFIGDIR}
)

##################################################
# Uninstall Target
##################################################
configure_file(
    "${CMAKE_CURRENT_SOURCE_DIR}/cmake_uninstall.cmake.in"
    "${CMAKE_CURRENT_BINARY_DIR}/cmake_uninstall.cmake"
    IMMEDIATE @ONLY)

add_custom_target(uninstall
    COMMAND ${CMAKE_COMMAND} -P ${CMAKE_CURRENT_BINARY_DIR}/cmake_uninstall.cmake)

##################################################
# Add subdirectories
##################################################
if(BUILD_BENCHMARKS)
  message(STATUS "Build benchmarks")
  add_subdirectory(benchmark)
endif()

if(BUILD_DOCS)
  message(STATUS "Build docs")
  add_subdirectory(doc)
endif()

if(BUILD_EXAMPLES)
  message(STATUS "Build examples")
  add_subdirectory(examples)
endif()

if(BUILD_PYTHON)
  message(STATUS "Build Python wrappers")
  add_subdirectory(wrappers/python)
endif()

if(BUILD_TESTING)
  message(STATUS "Build tests")
  add_subdirectory(test)
endif()

include(CPackConfig.cmake)
