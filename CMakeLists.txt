
cmake_minimum_required(VERSION 3.7)

project(CognitiveVRAnalytics)

set(BUILD_STATIC_LIBS TRUE)

if(MSVC)
  set(THIRDPARTY_CURL_DYNAMIC_IMPORT_LOCATION "${CMAKE_SOURCE_DIR}/ClientProject/ClientProject/lib/libcurl.dll" CACHE STRING "Library location")
  set(THIRDPARTY_CURL_STATIC_IMPORT_LOCATION "${CMAKE_SOURCE_DIR}/ClientProject/ClientProject/lib/libcurl.lib" CACHE STRING "Library location")
  set(THIRDPARTY_GTEST_STATIC_RELEASE_IMPORT_LOCATION "${CMAKE_SOURCE_DIR}/ClientProject/ClientProject/include/googletest/msvc/2010/gtest-md/Win32-Release/gtest.lib" CACHE STRING "Library location")
  
  set(THIRDPARTY_GTEST_STATIC_DEBUG_IMPORT_LOCATION "${CMAKE_SOURCE_DIR}/ClientProject/ClientProject/include/googletest/msvc/2010/gtest-md/Win32-Debug/gtestd.lib" CACHE STRING "Library location")
endif()

add_library(imported_curl STATIC IMPORTED)
set_target_properties(imported_curl PROPERTIES IMPORTED_LOCATION "${THIRDPARTY_CURL_STATIC_IMPORT_LOCATION}")


file(GLOB cognitive3d_libs_SRCS
  ${CMAKE_SOURCE_DIR}/CognitiveVRAnalytics/CognitiveVRAnalytics/*.c*
  )

file(GLOB cognitive3d_libs_HDRS
  ${CMAKE_SOURCE_DIR}/CognitiveVRAnalytics/CognitiveVRAnalytics/*.h*
  )

if(BUILD_STATIC_LIBS)

  add_library(${CMAKE_PROJECT_NAME}-static STATIC ${cognitive3d_libs_SRCS} ${cognitive3d_libs_HDRS})

  target_link_libraries(${CMAKE_PROJECT_NAME}-static imported_curl)

  target_include_directories(${CMAKE_PROJECT_NAME}-static 
    PUBLIC 
    ${CMAKE_SOURCE_DIR}
    ${CMAKE_SOURCE_DIR}/CognitiveVRAnalytics/CognitiveVRAnalytics 
    ${CMAKE_SOURCE_DIR}/ClientProject/ClientProject/include
    ${CMAKE_SOURCE_DIR}/ClientProject/ClientProject/include/googletest/include
    )

  install(TARGETS ${CMAKE_PROJECT_NAME}-static DESTINATION ${CMAKE_BINARY_DIR}/install/lib/${CMAKE_BUILD_TYPE} )

  install(FILES ${CMAKE_SOURCE_DIR}/CognitiveVRAnalytics/CognitiveVRAnalytics/cognitive_log.h DESTINATION ${CMAKE_BINARY_DIR}/install/include)
  install(FILES ${CMAKE_SOURCE_DIR}/CognitiveVRAnalytics/CognitiveVRAnalytics/CognitiveVRAnalytics.h DESTINATION ${CMAKE_BINARY_DIR}/install/include)
  install(FILES ${CMAKE_SOURCE_DIR}/CognitiveVRAnalytics/CognitiveVRAnalytics/config.h DESTINATION ${CMAKE_BINARY_DIR}/install/include)
  install(FILES ${CMAKE_SOURCE_DIR}/CognitiveVRAnalytics/CognitiveVRAnalytics/coresettings.h DESTINATION ${CMAKE_BINARY_DIR}/install/include)
  install(FILES ${CMAKE_SOURCE_DIR}/CognitiveVRAnalytics/CognitiveVRAnalytics/customevent.h DESTINATION ${CMAKE_BINARY_DIR}/install/include)
  install(FILES ${CMAKE_SOURCE_DIR}/CognitiveVRAnalytics/CognitiveVRAnalytics/dynamicobject.h DESTINATION ${CMAKE_BINARY_DIR}/install/include)
  install(FILES ${CMAKE_SOURCE_DIR}/CognitiveVRAnalytics/CognitiveVRAnalytics/exitpoll.h DESTINATION ${CMAKE_BINARY_DIR}/install/include)
  install(FILES ${CMAKE_SOURCE_DIR}/CognitiveVRAnalytics/CognitiveVRAnalytics/gazetracker.h DESTINATION ${CMAKE_BINARY_DIR}/install/include)
  install(FILES ${CMAKE_SOURCE_DIR}/CognitiveVRAnalytics/CognitiveVRAnalytics/json.hpp DESTINATION ${CMAKE_BINARY_DIR}/install/include)
  install(FILES ${CMAKE_SOURCE_DIR}/CognitiveVRAnalytics/CognitiveVRAnalytics/network.h DESTINATION ${CMAKE_BINARY_DIR}/install/include)
  install(FILES ${CMAKE_SOURCE_DIR}/CognitiveVRAnalytics/CognitiveVRAnalytics/sensor.h DESTINATION ${CMAKE_BINARY_DIR}/install/include)
  install(FILES ${CMAKE_SOURCE_DIR}/CognitiveVRAnalytics/CognitiveVRAnalytics/stdafx.h DESTINATION ${CMAKE_BINARY_DIR}/install/include)

endif()

if(TARGET ${CMAKE_PROJECT_NAME}-static)
  add_library(imported_gtest_release STATIC IMPORTED)
  set_target_properties(imported_gtest_release PROPERTIES IMPORTED_LOCATION "${THIRDPARTY_GTEST_STATIC_RELEASE_IMPORT_LOCATION}")

  add_library(imported_gtest_debug STATIC IMPORTED)
  set_target_properties(imported_gtest_debug PROPERTIES IMPORTED_LOCATION "${THIRDPARTY_GTEST_STATIC_DEBUG_IMPORT_LOCATION}")

  file(GLOB cognitive3d_exe_SRCS
    ${CMAKE_SOURCE_DIR}/ClientProject/ClientProject/*.c*
    )

  file(GLOB cognitive3d_exe_HDRS
    ${CMAKE_SOURCE_DIR}/ClientProject/ClientProject/*.h*
    )

  add_executable(${CMAKE_PROJECT_NAME}-test-exe ${cognitive3d_exe_SRCS} ${cognitive3d_exe_HDRS})
  target_include_directories(${CMAKE_PROJECT_NAME}-test-exe 
    PUBLIC 
    ${CMAKE_SOURCE_DIR}
    ${CMAKE_SOURCE_DIR}/CognitiveVRAnalytics/CognitiveVRAnalytics
    ${CMAKE_SOURCE_DIR}/ClientProject/ClientProject/include
    ${CMAKE_SOURCE_DIR}/ClientProject/ClientProject/include/googletest/include
    )
  target_link_libraries(${CMAKE_PROJECT_NAME}-test-exe ${CMAKE_PROJECT_NAME}-static imported_curl)
  target_link_libraries(${CMAKE_PROJECT_NAME}-test-exe optimized imported_gtest_release debug imported_gtest_debug)

  set_property(DIRECTORY ${CMAKE_SOURCE_DIR} PROPERTY VS_STARTUP_PROJECT ${CMAKE_PROJECT_NAME}-test-exe)

  add_custom_command(TARGET ${CMAKE_PROJECT_NAME}-test-exe POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy
    ${THIRDPARTY_CURL_DYNAMIC_IMPORT_LOCATION}
    $<TARGET_FILE_DIR:${CMAKE_PROJECT_NAME}-test-exe>
    )

endif()

