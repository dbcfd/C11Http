set (TARGET Objects)
add_definitions(-DOBJECTS_BUILD)

file(GLOB HEADERS "*.h")

file(GLOB SOURCES "*.cpp")

add_library (${TARGET} SHARED ${HEADERS} ${SOURCES}) 
target_link_libraries (${TARGET} ${DEPENDENCIES})

set_target_properties (${TARGET} PROPERTIES
    SOVERSION ${PROJECT_SO_VERSION})

install (TARGETS ${TARGET} RUNTIME DESTINATION bin LIBRARY DESTINATION lib ARCHIVE DESTINATION lib)

SetVSTargetProperties(${TARGET})