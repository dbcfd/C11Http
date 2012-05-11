add_subdirectory(interface)
add_subdirectory(posix)
add_subdirectory(windows)

set (TARGET Client)
add_definitions(-DCLIENT_BUILD)

file(GLOB HEADERS "*.h")

file(GLOB SOURCES "*.cpp")

if(UNIX)
	set(DEPENDENCIES ${DEPENDENCIES} rt)
endif()	

if(UNIX)
	SET (DEPENDENCIES ${DEPENDENCIES} ClientPosix)
else()
	SET (DEPENDENCIES ${DEPENDENCIES} ClientWindows)
endif()

add_library (${TARGET} SHARED ${HEADERS} ${SOURCES}) 
target_link_libraries (${TARGET} ${DEPENDENCIES})

set_target_properties (${TARGET} PROPERTIES
    SOVERSION ${PROJECT_SO_VERSION})

install (TARGETS ${TARGET} RUNTIME DESTINATION bin LIBRARY DESTINATION lib ARCHIVE DESTINATION lib)

SetVSTargetProperties(${TARGET})