set (TARGET ServerPosix)
add_definitions(-DSERVER_POSIX_BUILD)

file(GLOB_RECURSE HEADERS "*.h")

file(GLOB_RECURSE SOURCES "*.cpp")

set(DEPENDENCIES ${DEPENDENCIES} ServerInterface)

if(UNIX)
	set(DEPENDENCIES ${DEPENDENCIES} rt)
endif()	

add_library (${TARGET} SHARED ${HEADERS} ${SOURCES}) 
target_link_libraries (${TARGET} ${DEPENDENCIES})

set_target_properties (${TARGET} PROPERTIES
    SOVERSION ${PROJECT_SO_VERSION})

install (TARGETS ${TARGET} RUNTIME DESTINATION bin LIBRARY DESTINATION lib ARCHIVE DESTINATION lib)

SetVSTargetProperties(${TARGET})
