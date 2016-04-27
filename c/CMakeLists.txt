# Create AMCL library and examples

# add the binary tree directory to the search path for include files and linking
include_directories( ${CMAKE_CURRENT_BINARY_DIR} )
link_directories( ${CMAKE_CURRENT_BINARY_DIR} )

# post build: mv amcl_.h amcl.h
if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/amcl.h")
  file(RENAME amcl.h amcl_.h)
endif()
file(COPY DLLDefines.h DESTINATION ${CMAKE_CURRENT_BINARY_DIR})

set(SOURCES_AMCL
big.c
fp.c
ecp.c
hash.c
rand.c
aes.c
gcm.c
oct.c
rom.c
fp2.c
ecp2.c
fp4.c
fp12.c
ff.c)

# Build AMCL
add_library(amcl ${LIB_TYPE} ${SOURCES_AMCL} version.c)

if(${CMAKE_SYSTEM_NAME} MATCHES "Linux")
  if(BUILD_SHARED_LIBS)
    message(STATUS "Build shared libs")
    target_link_libraries (amcl libm.so) 
  else()
    message(STATUS "Build static libs")
    target_link_libraries (amcl libm.a) 
  endif(BUILD_SHARED_LIBS)
endif(${CMAKE_SYSTEM_NAME} MATCHES "Linux")

# Build libmpin
if(BUILD_MPIN)
  add_library(mpin ${LIB_TYPE} pair.c mpin.c utils.c)
  target_link_libraries (mpin amcl) 
  add_executable (testmpin testmpin.c)
  target_link_libraries (testmpin mpin) 
endif(BUILD_MPIN)

# Build libwcc
if(BUILD_WCC)
  message(STATUS "Build WCC")
  add_library(wcc ${LIB_TYPE} pair.c wcc.c utils.c)
  target_link_libraries (wcc amcl) 
  add_executable (testwcc testwcc.c)
  add_executable (testwcc_tp testwcc_tp.c)
  add_executable (testwcc_dta testwcc_dta.c)
  target_link_libraries (testwcc wcc) 
  target_link_libraries (testwcc_tp wcc) 
  target_link_libraries (testwcc_dta wcc) 
endif(BUILD_WCC)

# Copy built libs to c/tests which allows testing without
# having to install the libs.
if(${CMAKE_SYSTEM_NAME} MATCHES "Windows")
    message(STATUS "Copy libamcl.dll to c/tests for testing")
    add_custom_command(
                TARGET amcl
                POST_BUILD
                COMMAND ${CMAKE_COMMAND} -E copy "${CMAKE_CURRENT_BINARY_DIR}/libamcl.dll" "${PROJECT_BINARY_DIR}/c/tests/")

    if(BUILD_MPIN)
      message(STATUS "Copy libmpin.dll to c/tests for testing")
      add_custom_command(
                  TARGET mpin
                  POST_BUILD
                  COMMAND ${CMAKE_COMMAND} -E copy "${CMAKE_CURRENT_BINARY_DIR}/libmpin.dll" "${PROJECT_BINARY_DIR}/c/tests/")
    endif(BUILD_MPIN)

    if(BUILD_WCC)
      message(STATUS "Copy libwcc.dll to c/tests for testing")
      add_custom_command(
                  TARGET wcc
                  POST_BUILD
                  COMMAND ${CMAKE_COMMAND} -E copy "${CMAKE_CURRENT_BINARY_DIR}/libwcc.dll" "${PROJECT_BINARY_DIR}/c/tests/")
    endif(BUILD_WCC)

endif(${CMAKE_SYSTEM_NAME} MATCHES "Windows")

# Copy built libs to pythonCFFI which allows testing without
# having to install the libs.
if (BUILD_PYTHON AND BUILD_SHARED_LIBS)

  if(${CMAKE_SYSTEM_NAME} MATCHES "Linux")

    if(BUILD_MPIN)
      message(STATUS "Copy libmpin.so to pythonCFFI for testing")
      add_custom_command(
                  TARGET mpin
                  POST_BUILD
                  COMMAND ${CMAKE_COMMAND} -E copy "${CMAKE_CURRENT_BINARY_DIR}/libmpin.so" "${PROJECT_BINARY_DIR}/pythonCFFI/")
    endif(BUILD_MPIN)

    if(BUILD_WCC)
      message(STATUS "Copy libwcc.so to pythonCFFI for testing")
      add_custom_command(
                  TARGET wcc
                  POST_BUILD
                  COMMAND ${CMAKE_COMMAND} -E copy "${CMAKE_CURRENT_BINARY_DIR}/libwcc.so" "${PROJECT_BINARY_DIR}/pythonCFFI/")
    endif(BUILD_WCC)  

  endif(${CMAKE_SYSTEM_NAME} MATCHES "Linux")

  if(${CMAKE_SYSTEM_NAME} MATCHES "Windows")
    message(STATUS "Copy libamcl.dll to pythonCFFI for testing")
    add_custom_command(
                TARGET amcl
                POST_BUILD
                COMMAND ${CMAKE_COMMAND} -E copy "${CMAKE_CURRENT_BINARY_DIR}/libamcl.dll" "${PROJECT_BINARY_DIR}/pythonCFFI/")

    if(BUILD_MPIN)
      message(STATUS "Copy libmpin.dll to pythonCFFI for testing")
      add_custom_command(
                  TARGET mpin
                  POST_BUILD
                  COMMAND ${CMAKE_COMMAND} -E copy "${CMAKE_CURRENT_BINARY_DIR}/libmpin.dll" "${PROJECT_BINARY_DIR}/pythonCFFI/")
    endif(BUILD_MPIN)

    if(BUILD_WCC)
      message(STATUS "Copy libwcc.dll to pythonCFFI for testing")
      add_custom_command(
                  TARGET wcc
                  POST_BUILD
                  COMMAND ${CMAKE_COMMAND} -E copy "${CMAKE_CURRENT_BINARY_DIR}/libwcc.dll" "${PROJECT_BINARY_DIR}/pythonCFFI/")
    endif(BUILD_WCC)

  endif(${CMAKE_SYSTEM_NAME} MATCHES "Windows")

  if(${CMAKE_SYSTEM_NAME} MATCHES "Darwin")

    message(STATUS "Copy libamcl.dylib to pythonCFFI for testing")
    add_custom_command(
                TARGET amcl
                POST_BUILD
                COMMAND ${CMAKE_COMMAND} -E copy "${CMAKE_CURRENT_BINARY_DIR}/libamcl.dylib" "${PROJECT_BINARY_DIR}/pythonCFFI/")

    if(BUILD_MPIN)
      message(STATUS "Copy libmpin.dylib to pythonCFFI for testing")
      add_custom_command(
                  TARGET mpin
                  POST_BUILD
                  COMMAND ${CMAKE_COMMAND} -E copy "${CMAKE_CURRENT_BINARY_DIR}/libmpin.dylib" "${PROJECT_BINARY_DIR}/pythonCFFI/")
    endif(BUILD_MPIN)

    if(BUILD_WCC)
      message(STATUS "Copy libwcc.dylib to pythonCFFI for testing")
      add_custom_command(
                  TARGET wcc
                  POST_BUILD
                  COMMAND ${CMAKE_COMMAND} -E copy "${CMAKE_CURRENT_BINARY_DIR}/libwcc.dylib" "${PROJECT_BINARY_DIR}/pythonCFFI/")
    endif(BUILD_WCC)  


  endif(${CMAKE_SYSTEM_NAME} MATCHES "Darwin")


endif (BUILD_PYTHON AND BUILD_SHARED_LIBS)

# Example programs
add_executable (testecm testecm.c ecdh.c)
add_executable (testecdh testecdh.c ecdh.c)
add_executable (testrsa testrsa.c rsa.c)
target_link_libraries (testecm amcl) 
target_link_libraries (testecdh amcl) 
target_link_libraries (testrsa amcl) 

add_subdirectory (tests)

# Install libs and headers
if(${CMAKE_SYSTEM_NAME} MATCHES "Linux")
  install(DIRECTORY DESTINATION include DIRECTORY_PERMISSIONS
          OWNER_WRITE OWNER_READ OWNER_EXECUTE
          GROUP_WRITE GROUP_READ
          WORLD_WRITE WORLD_READ WORLD_EXECUTE)
  install (TARGETS amcl DESTINATION lib PERMISSIONS
          OWNER_WRITE OWNER_READ OWNER_EXECUTE
          GROUP_READ GROUP_EXECUTE
          WORLD_READ WORLD_EXECUTE)
  install (FILES ${CMAKE_CURRENT_BINARY_DIR}/amcl.h DESTINATION include PERMISSIONS
          OWNER_WRITE OWNER_READ OWNER_EXECUTE
          GROUP_READ GROUP_EXECUTE
          WORLD_READ WORLD_EXECUTE)
  install (FILES ${CMAKE_CURRENT_SOURCE_DIR}/utils.h DESTINATION include PERMISSIONS
          OWNER_WRITE OWNER_READ OWNER_EXECUTE
          GROUP_READ GROUP_EXECUTE
          WORLD_READ WORLD_EXECUTE)
  install (FILES ${CMAKE_CURRENT_SOURCE_DIR}/DLLDefines.h DESTINATION include PERMISSIONS
          OWNER_WRITE OWNER_READ OWNER_EXECUTE
          GROUP_READ GROUP_EXECUTE
          WORLD_READ WORLD_EXECUTE)

  if(BUILD_MPIN)
    install (TARGETS mpin DESTINATION lib PERMISSIONS
            OWNER_WRITE OWNER_READ OWNER_EXECUTE
            GROUP_READ GROUP_EXECUTE
            WORLD_READ WORLD_EXECUTE)
    install (FILES ${CMAKE_CURRENT_SOURCE_DIR}/mpin.h DESTINATION include PERMISSIONS
            OWNER_WRITE OWNER_READ OWNER_EXECUTE
            GROUP_READ GROUP_EXECUTE
            WORLD_READ WORLD_EXECUTE)
  endif(BUILD_MPIN)
  
  if(BUILD_WCC)
    install (TARGETS wcc DESTINATION lib PERMISSIONS
            OWNER_WRITE OWNER_READ OWNER_EXECUTE
            GROUP_READ GROUP_EXECUTE
            WORLD_READ WORLD_EXECUTE)
    install (FILES ${CMAKE_CURRENT_SOURCE_DIR}/wcc.h DESTINATION include PERMISSIONS
            OWNER_WRITE OWNER_READ OWNER_EXECUTE
            GROUP_READ GROUP_EXECUTE
            WORLD_READ WORLD_EXECUTE)
  endif(BUILD_WCC)

endif(${CMAKE_SYSTEM_NAME} MATCHES "Linux")

if(${CMAKE_SYSTEM_NAME} MATCHES "Windows")
  # Windows specific code
  install (TARGETS amcl DESTINATION lib)
  install (FILES ${CMAKE_CURRENT_BINARY_DIR}/amcl.h DESTINATION lib)
  install (FILES ${CMAKE_CURRENT_SOURCE_DIR}/utils.h DESTINATION lib)
  install (FILES ${CMAKE_CURRENT_SOURCE_DIR}/DLLDefines.h DESTINATION lib)
  
  if(BUILD_MPIN)
    install (TARGETS mpin DESTINATION lib)
    install (FILES ${CMAKE_CURRENT_SOURCE_DIR}/mpin.h DESTINATION lib)
  endif(BUILD_MPIN)
  
  if(BUILD_WCC)
    install (TARGETS wcc DESTINATION lib)
    install (FILES ${CMAKE_CURRENT_SOURCE_DIR}/wcc.h DESTINATION lib)
  endif(BUILD_WCC)

endif(${CMAKE_SYSTEM_NAME} MATCHES "Windows")

if(${CMAKE_SYSTEM_NAME} MATCHES "Darwin")
  # Mac OS X specific code
  install(DIRECTORY DESTINATION include DIRECTORY_PERMISSIONS
          OWNER_WRITE OWNER_READ OWNER_EXECUTE
          GROUP_WRITE GROUP_READ
          WORLD_WRITE WORLD_READ WORLD_EXECUTE)
  install (TARGETS amcl DESTINATION lib PERMISSIONS
          OWNER_WRITE OWNER_READ OWNER_EXECUTE
          GROUP_READ GROUP_EXECUTE
          WORLD_READ WORLD_EXECUTE)
  install (FILES ${CMAKE_CURRENT_BINARY_DIR}/amcl.h DESTINATION include PERMISSIONS
          OWNER_WRITE OWNER_READ OWNER_EXECUTE
          GROUP_READ GROUP_EXECUTE
          WORLD_READ WORLD_EXECUTE)
  install (FILES ${CMAKE_CURRENT_SOURCE_DIR}/utils.h DESTINATION include PERMISSIONS
          OWNER_WRITE OWNER_READ OWNER_EXECUTE
          GROUP_READ GROUP_EXECUTE
          WORLD_READ WORLD_EXECUTE)
  install (FILES ${CMAKE_CURRENT_SOURCE_DIR}/DLLDefines.h DESTINATION include PERMISSIONS
          OWNER_WRITE OWNER_READ OWNER_EXECUTE
          GROUP_READ GROUP_EXECUTE
          WORLD_READ WORLD_EXECUTE)

  if(BUILD_MPIN)
    install (TARGETS mpin DESTINATION lib PERMISSIONS
            OWNER_WRITE OWNER_READ OWNER_EXECUTE
            GROUP_READ GROUP_EXECUTE
            WORLD_READ WORLD_EXECUTE)
    install (FILES ${CMAKE_CURRENT_SOURCE_DIR}/mpin.h DESTINATION include PERMISSIONS
            OWNER_WRITE OWNER_READ OWNER_EXECUTE
            GROUP_READ GROUP_EXECUTE
            WORLD_READ WORLD_EXECUTE)
  endif(BUILD_MPIN)

  if(BUILD_WCC)
    install (TARGETS wcc DESTINATION lib PERMISSIONS
            OWNER_WRITE OWNER_READ OWNER_EXECUTE
            GROUP_READ GROUP_EXECUTE
            WORLD_READ WORLD_EXECUTE)
    install (FILES ${CMAKE_CURRENT_SOURCE_DIR}/wcc.h DESTINATION include PERMISSIONS
            OWNER_WRITE OWNER_READ OWNER_EXECUTE
            GROUP_READ GROUP_EXECUTE
            WORLD_READ WORLD_EXECUTE)
  endif(BUILD_WCC)


endif(${CMAKE_SYSTEM_NAME} MATCHES "Darwin")
