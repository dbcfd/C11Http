cmake_minimum_required(VERSION 2.8.4)

project(C11Http)

set(version 0.1.0)

if(NOT CMAKE_BUILD_TYPE)
	set(CMAKE_BUILD_TYPE Debug)
endif()

if(UNIX)
	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -DUNIX")
else()
	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -DWINDOWS")
endif()

set(PROJECT_SO_VERSION ${version})

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} ${CMAKE_CURRENT_SOURCE_DIR}/cmakeModules)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/${CMAKE_BUILD_TYPE}/lib)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/${CMAKE_BUILD_TYPE}/bin)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/${CMAKE_BUILD_TYPE}/lib)

include(SetVSTargetProperties)

include_directories(${CMAKE_CURRENT_SOURCE_DIR})
set(BASE_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR})

add_subdirectory(objects)
add_subdirectory(workers)
add_subdirectory(server)
add_subdirectory(client)
add_subdirectory(thirdparty)
add_subdirectory(test)