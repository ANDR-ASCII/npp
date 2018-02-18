cmake_minimum_required(VERSION 2.8.11)
project(${AppName})

set(AppSourcesDir src)
set(AppHeadersDir include)

aux_source_directory(${AppSourcesDir} AppSourcesList)
file(GLOB_RECURSE AppCppList "src/*.cpp")
file(GLOB_RECURSE AppHeadersList "include/*.h")

list(REMOVE_ITEM AppSourcesList src/stdafx.cpp)

add_msvc_precompiled_header("stdafx.h" "src/stdafx.cpp" AppSourcesList)

source_group("Precompiled Headers" FILES include/stdafx.h src/stdafx.cpp)

add_executable(
	${AppName} 
	${AppSourcesList} 
	${AppHeadersList} 
)

if(MSVC)
	add_definitions(
		/EHsc
		/MP
		/permissive-
		/std:c++latest
	)
	
	set_target_properties(
		${AppName}
		PROPERTIES COMPILE_FLAGS "/FI\"stdafx.h\""
	)
	
	set_target_properties(
		${AppName} 
		PROPERTIES WIN32_EXECUTABLE YES
		LINK_FLAGS "/ENTRY:mainCRTStartup"
	)
endif()

include_directories(
	${AppHeadersDir}
	${CMAKE_CURRENT_BINARY_DIR}
)

target_link_libraries(
	${AppName}
)