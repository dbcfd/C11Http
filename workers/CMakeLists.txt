set (TARGET Workers)
add_definitions(-DWORKERS_BUILD)

file(GLOB HEADERS "*.h")

file(GLOB SOURCES "*.cpp")

set(DEPENDENCIES ${DEPENDENCIES} Objects)

if(UNIX)
	set(DEPENDENCIES ${DEPENDENCIES} rt)
endif()	

add_library (${TARGET} SHARED ${HEADERS} ${SOURCES}) 
target_link_libraries (${TARGET} ${DEPENDENCIES})

set_target_properties (${TARGET} PROPERTIES
    SOVERSION ${PROJECT_SO_VERSION})

install (TARGETS ${TARGET} RUNTIME DESTINATION bin LIBRARY DESTINATION lib ARCHIVE DESTINATION lib)

SetVSTargetProperties(${TARGET})