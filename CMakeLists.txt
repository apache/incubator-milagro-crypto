cmake_minimum_required (VERSION 2.8)
project (AMCL)
enable_testing()

set(AMCL_VERSION_MAJOR "1")
set(AMCL_VERSION_MINOR "0")
set(AMCL_VERSION_PATCH "0")
set(PROJECT_VERSION "${AMCL_VERSION_MAJOR}.${AMCL_VERSION_MINOR}.${AMCL_VERSION_PATCH}")
message(STATUS "PROJECT_VERSION='${PROJECT_VERSION}'")
message(STATUS "Generator - ${CMAKE_GENERATOR}")

# Add options for build
option (BUILD_SHARED_LIBS "Build Shared Libraries" ON)
option (BUILD_PYTHON "Build Python" ON) 
option (BUILD_MPIN "Build MPIN" ON) 
option (BUILD_DOXYGEN "Build Doxygen" ON) 
option (USE_PATENTS "Use alleged patents" ON)

# Allow the developer to select if Dynamic or Static libraries are built
# Set the default LIB_TYPE variable to STATIC
SET (LIB_TYPE STATIC)
IF (BUILD_SHARED_LIBS)
  # User wants to build Dynamic Libraries, so change the LIB_TYPE variable to CMake keyword 'SHARED'
  SET (LIB_TYPE SHARED)
ENDIF (BUILD_SHARED_LIBS)

if(${CMAKE_SYSTEM_NAME} MATCHES "Darwin")
   # Mac OS X specific code
   set(OS "Mac OS X")
   # MACOSX_RPATH
   cmake_policy(SET CMP0042 NEW)
   set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -std=c99")
endif(${CMAKE_SYSTEM_NAME} MATCHES "Darwin")

if(${CMAKE_SYSTEM_NAME} MATCHES "Linux")
   # Linux specific code
   set(OS "Linux")
   set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -std=c99")
endif(${CMAKE_SYSTEM_NAME} MATCHES "Linux")

if(${CMAKE_SYSTEM_NAME} MATCHES "Windows")
   # Windows specific code
   set(OS "Windows")
endif(${CMAKE_SYSTEM_NAME} MATCHES "Windows")

# Select word length
set(WORD_LENGTH "32" CACHE STRING "Word length. See amcl.h.in for explanation")
set(Word_Length_Values "16;32;64")
set_property(CACHE WORD_LENGTH PROPERTY STRINGS ${Word_Length_Values})
message(STATUS "WORD_LENGTH='${WORD_LENGTH}'")

# Select Field
set(FIELD_CHOICE "BNCX" CACHE STRING "Choice of Field. See amcl.h.in for explanation")
set(Field_Values "BN;BNCX;BNT;BNT2;NIST;C25519;BRAINPOOL;ANSSI;MF254;MS255;MF256;MS256")
set_property(CACHE FIELD_CHOICE PROPERTY STRINGS ${Field_Values})
message(STATUS "FIELD_CHOICE='${FIELD_CHOICE}'")

# Select Curve type
set(CURVE_TYPE "WEIERSTRASS" CACHE STRING "Choice of Curve. See amcl.h.in for explanation")
set(Curve_Values "WEIERSTRASS;EDWARDS;MONTGOMERY")
set_property(CACHE CURVE_TYPE PROPERTY STRINGS ${Curve_Values})
message(STATUS "CURVE_TYPE='${CURVE_TYPE}'")

# configure amcl header file to pass CMake settings
# to the source code
configure_file (
  "${PROJECT_SOURCE_DIR}/c/amcl.h.in"
  "${PROJECT_BINARY_DIR}/c/amcl.h"
  )

if(NOT(FIELD_CHOICE MATCHES "BN"))
  message(STATUS "Field choice prevents MPIN being built")
  set(BUILD_MPIN OFF)
endif(NOT(FIELD_CHOICE MATCHES "BN"))

# test configuration
if(BUILD_MPIN)
  set(MPIN_TIME_PERMIT_TESTS 10 CACHE STRING "Number of days in the future to test M-PIN time permits")
  set(MPIN_RANDOM_TESTS 10 CACHE STRING "Number of random M-PIN tests")
  message(STATUS "MPIN_TIME_PERMIT_TESTS=${MPIN_TIME_PERMIT_TESTS}")
  message(STATUS "MPIN_RANDOM_TESTS=${MPIN_RANDOM_TESTS}")
  # Configure test variables
  configure_file (
    "${PROJECT_SOURCE_DIR}/c/tests/config.h.in"
    "${PROJECT_BINARY_DIR}/c/tests/config.h"
  )
endif(BUILD_MPIN)

# Set a default build type if none was specified
if(NOT CMAKE_BUILD_TYPE AND NOT CMAKE_CONFIGURATION_TYPES)
  message(STATUS "Setting build type to 'Release' as none was specified.")
  set(CMAKE_BUILD_TYPE Release CACHE STRING "Choose the type of build." FORCE)
  # Set the possible values of build type for cmake-gui
  set_property(CACHE CMAKE_BUILD_TYPE PROPERTY STRINGS "Debug" "Release" "MinSizeRel" "RelWithDebInfo")
endif()

# Add subdirectories
add_subdirectory (c)

# Only build Python for shared libs
if (NOT BUILD_SHARED_LIBS)
  set(BUILD_PYTHON OFF)
endif (NOT BUILD_SHARED_LIBS)

if (BUILD_PYTHON)
  message(STATUS "Build Python CFFI")
  execute_process (COMMAND python -c "from distutils.sysconfig import get_python_lib; print get_python_lib()" OUTPUT_VARIABLE PYTHON_SITE_PACKAGES OUTPUT_STRIP_TRAILING_WHITESPACE)
  execute_process (COMMAND python -c "from distutils.sysconfig import get_python_lib; from os.path import dirname; print dirname(get_python_lib())" OUTPUT_VARIABLE PYTHON_SITE_LIB OUTPUT_STRIP_TRAILING_WHITESPACE)

  if((FIELD_CHOICE STREQUAL "BNCX") AND BUILD_MPIN)
    file(COPY "${PROJECT_SOURCE_DIR}/testVectors/mpin/BNCX.json" DESTINATION "${PROJECT_BINARY_DIR}/pythonCFFI")
    file(RENAME "${PROJECT_BINARY_DIR}/pythonCFFI/BNCX.json" "${PROJECT_BINARY_DIR}/pythonCFFI/MPINTestVectors.json")
  endif((FIELD_CHOICE STREQUAL "BNCX") AND BUILD_MPIN)
  add_subdirectory (pythonCFFI)
endif (BUILD_PYTHON)

# Build Documentation
if (BUILD_DOXYGEN)
  find_package(Doxygen QUIET)
  if(DOXYGEN_FOUND)
    add_subdirectory (docs)
  else(DOXYGEN_FOUND)
    message(STATUS "Doxygen not found. Documentation will not be built.")
  endif(DOXYGEN_FOUND)
endif (BUILD_DOXYGEN)

# uninstall target
configure_file(
    "${CMAKE_CURRENT_SOURCE_DIR}/cmake_uninstall.cmake.in"
    "${CMAKE_CURRENT_BINARY_DIR}/cmake_uninstall.cmake"
    IMMEDIATE @ONLY)

add_custom_target(uninstall
    COMMAND ${CMAKE_COMMAND} -P ${CMAKE_CURRENT_BINARY_DIR}/cmake_uninstall.cmake)

# Build a CPack driven installer package
include (InstallRequiredSystemLibraries)
set(CPACK_RESOURCE_FILE_LICENSE "${CMAKE_CURRENT_SOURCE_DIR}/LICENSE")
set(CPACK_PACKAGE_VERSION "${AMCL_VERSION_MAJOR}.${AMCL_VERSION_MINOR}.${AMCL_VERSION_PATCH}")
set(CPACK_PACKAGE_NAME "AMCL")
set(CPACK_PACKAGE_RELEASE 1)
set(CPACK_PACKAGE_CONTACT "support@miracl.com")
set(CPACK_PACKAGE_VENDOR "MIRACL")
if (BUILD_PYTHON)
  set(CPACK_RPM_PACKAGE_REQUIRES "python >= 2.7.0")
endif (BUILD_PYTHON)
set(CPACK_PACKAGE_FILE_NAME "${CPACK_PACKAGE_NAME}-${CPACK_PACKAGE_VERSION}-${CPACK_PACKAGE_RELEASE}.${CMAKE_SYSTEM_PROCESSOR}")

if(${CMAKE_SYSTEM_NAME} MATCHES "Linux")
  # Linux specific code
  set(CPACK_PACKAGING_INSTALL_PREFIX ${CMAKE_INSTALL_PREFIX})
  # Prevents CPack from generating file conflicts
  set(CPACK_RPM_EXCLUDE_FROM_AUTO_FILELIST_ADDITION "${CPACK_PACKAGING_INSTALL_PREFIX}")
  list(APPEND CPACK_RPM_EXCLUDE_FROM_AUTO_FILELIST_ADDITION "${CPACK_PACKAGING_INSTALL_PREFIX}/bin")
  list(APPEND CPACK_RPM_EXCLUDE_FROM_AUTO_FILELIST_ADDITION "${CPACK_PACKAGING_INSTALL_PREFIX}/include")
  list(APPEND CPACK_RPM_EXCLUDE_FROM_AUTO_FILELIST_ADDITION "${CPACK_PACKAGING_INSTALL_PREFIX}/lib")
  list(APPEND CPACK_RPM_EXCLUDE_FROM_AUTO_FILELIST_ADDITION "${PYTHON_SITE_LIB}")
  list(APPEND CPACK_RPM_EXCLUDE_FROM_AUTO_FILELIST_ADDITION "${PYTHON_SITE_PACKAGES}")
  set(CPACK_GENERATOR "RPM")
endif(${CMAKE_SYSTEM_NAME} MATCHES "Linux")

if(${CMAKE_SYSTEM_NAME} MATCHES "Windows")
  # Windows specific code
  set(CPACK_PACKAGE_INSTALL_DIRECTORY "AMCL")
  set(CPACK_NSIS_MODIFY_PATH ON)
  set(CPACK_PACKAGE_ICON "${CMAKE_SOURCE_DIR}/data\\\\icon.bmp")
  set(CPACK_NSIS_MUI_ICON "${CMAKE_SOURCE_DIR}/data\\\\icon.ico")
  set(CPACK_NSIS_MUI_UNIICON "${CMAKE_SOURCE_DIR}/data\\\\icon.ico")
  set(CPACK_NSIS_HELP_LINK "http:\\\\\\\\www.miracl.com")
  set(CPACK_NSIS_URL_INFO_ABOUT "http:\\\\\\\\www.miracl.com")
  set(CPACK_NSIS_CONTACT "support@miracl.com")
endif(${CMAKE_SYSTEM_NAME} MATCHES "Windows")

if(${CMAKE_SYSTEM_NAME} MATCHES "Darwin")
  # Mac OS X specific code
endif(${CMAKE_SYSTEM_NAME} MATCHES "Darwin")

include (CPack)

