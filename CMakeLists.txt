cmake_minimum_required(VERSION 2.8.11)

set(AppName app)
project(${AppName})

set(CMAKE_INSTALL_PREFIX)

# Find includes in corresponding build directories
set(CMAKE_INCLUDE_CURRENT_DIR ON)

MACRO(add_msvc_precompiled_header PrecompiledHeader PrecompiledSource SourcesVar)
  IF(MSVC)
    GET_FILENAME_COMPONENT(PrecompiledBasename ${PrecompiledHeader} NAME_WE)
    SET(PrecompiledBinary "$(IntDir)/${PrecompiledBasename}.pch")
    SET(Sources ${${SourcesVar}})

    SET_SOURCE_FILES_PROPERTIES(
		${PrecompiledSource}
		PROPERTIES COMPILE_FLAGS "/Yc\"${PrecompiledHeader}\" /Fp\"${PrecompiledBinary}\""
		OBJECT_OUTPUTS "${PrecompiledBinary}"
	)
	   
    SET_SOURCE_FILES_PROPERTIES(
		${Sources}
        PROPERTIES COMPILE_FLAGS "/Yu\"${PrecompiledHeader}\" /FI\"${PrecompiledHeader}\" /Fp\"${PrecompiledBinary}\""
        OBJECT_DEPENDS "${PrecompiledBinary}"
	)  
		
    # Add precompiled header to SourcesVar
    LIST(APPEND ${SourcesVar} ${PrecompiledSource})
  ENDIF(MSVC)
ENDMACRO(add_msvc_precompiled_header)

add_subdirectory(${AppName})
