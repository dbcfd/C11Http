add_subdirectory(interface)
add_subdirectory(posix)
add_subdirectory(windows)

set (TARGET Tcp)
add_definitions(-DTCP_BUILD)

file(GLOB HEADERS "*.h")

file(GLOB SOURCES "*.cpp")

if(UNIX)
	set(DEPENDENCIES ${DEPENDENCIES} rt)
endif()	

if(UNIX)
	SET (DEPENDENCIES ${DEPENDENCIES} TcpPosix)
else()
	SET (DEPENDENCIES ${DEPENDENCIES} TcpWindows)
endif()

add_library (${TARGET} SHARED ${HEADERS} ${SOURCES}) 
target_link_libraries (${TARGET} ${DEPENDENCIES})

set_target_properties (${TARGET} PROPERTIES
    SOVERSION ${PROJECT_SO_VERSION})

install (TARGETS ${TARGET} RUNTIME DESTINATION bin LIBRARY DESTINATION lib ARCHIVE DESTINATION lib)

SetVSTargetProperties(${TARGET})