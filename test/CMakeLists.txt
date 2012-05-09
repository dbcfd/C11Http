add_definitions(-DGTEST_LINKED_AS_SHARED_LIBRARY)

set (TARGET UnitTesting)

file(GLOB_RECURSE SOURCES "*.cpp")

if(UNIX)
	set(DEPENDENCIES rt)
endif()	

set(gtest_dir ${BASE_DIRECTORY}/thirdparty/gtest-1.6.0/include)
include_directories(${gtest_dir})

SET (DEPENDENCIES ${DEPENDENCIES} Tcp gtest)

add_executable (${TARGET} ${HEADERS} ${SOURCES}) 
target_link_libraries (${TARGET} ${DEPENDENCIES})

install (TARGETS ${TARGET} RUNTIME DESTINATION bin LIBRARY DESTINATION lib ARCHIVE DESTINATION lib)

SetVSTargetProperties(${TARGET})