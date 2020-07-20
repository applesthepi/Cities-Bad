cmake_minimum_required(VERSION 3.10)

# use c++ 20
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED True)
set(CMAKE_POSITION_INDEPENDENT_CODE ON)
set(USE_FOLDERS ON)


project(citiesbad)

# Add X11 if using linux
# TODO: Proper wayland support
if (CMAKE_SYSTEM_NAME STREQUAL "Linux")
	set(LINUX True)
	message(STATUS "Compiling For Linux")
	find_package(X11 REQUIRED)
	# Might need to go in UNIX, ill check later
	find_package(TBB REQUIRED)
	list(APPEND EXTRA_LIBRARIES ${X11_LIBRARIES})
	list(APPEND EXTRA_INCLUDES ${X11_INCLUDE_DIRS})
endif()
# Not really posix, just a term given to every os that follows some standards (so everything besides windows)
if(UNIX)
	set(POSIX True)
endif()
set(CitiesBad_SRC
	# ==================================================================================
	# =============================================================== Headers
	# ==================================================================================
	
	# Root
	"${CMAKE_CURRENT_SOURCE_DIR}/CitiesBad/src/GameLayer.h"
	
	# Road
	"${CMAKE_CURRENT_SOURCE_DIR}/CitiesBad/src/road/Clip.h"
	"${CMAKE_CURRENT_SOURCE_DIR}/CitiesBad/src/road/ClipGroup.h"
	"${CMAKE_CURRENT_SOURCE_DIR}/CitiesBad/src/road/Lane.h"
	"${CMAKE_CURRENT_SOURCE_DIR}/CitiesBad/src/road/Vertex.h"
	# ==================================================================================
	# =============================================================== Source
	# ==================================================================================
	
	# Root
	"${CMAKE_CURRENT_SOURCE_DIR}/CitiesBad/src/main.cpp"
	"${CMAKE_CURRENT_SOURCE_DIR}/CitiesBad/src/GameLayer.cpp"
	
	# Road
	"${CMAKE_CURRENT_SOURCE_DIR}/CitiesBad/src/road/Clip.cpp"
	"${CMAKE_CURRENT_SOURCE_DIR}/CitiesBad/src/road/ClipGroup.cpp"
	"${CMAKE_CURRENT_SOURCE_DIR}/CitiesBad/src/road/Lane.cpp"
	"${CMAKE_CURRENT_SOURCE_DIR}/CitiesBad/src/road/Vertex.cpp"
	)
add_executable(citiesbad
	${CitiesBad_SRC}
	)
# Preserve folders in visual studio structure
source_group(TREE ${CMAKE_CURRENT_SOURCE_DIR}/CitiesBad/src FILES ${CitiesBad_SRC})

configure_file(config.h.in config.h)

set_property(TARGET citiesbad PROPERTY VS_DEBUGGER_WORKING_DIRECTORY "${CMAKE_BINARY_DIR}/Debug")
# Set unnamedblocks to the startup project
set_property(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} PROPERTY VS_STARTUP_PROJECT citiesbad)

add_subdirectory(OpenGLCore)

target_link_libraries(citiesbad PUBLIC
	OpenGLCore
	)
	
target_include_directories(citiesbad PUBLIC
	"OpenGLCore"
	"OpenGLCore/include"
	)

#target_link_libraries(unnamedblocks PUBLIC
#	RHR
#	Cappuccino
#	sfml-graphics
#	sfml-system
#	sfml-window
#	sfml-network
#	optimized "${csfml-audio}"
#    optimized "${csfml-network}"
#    optimized "${csfml-graphics}"
#    optimized "${csfml-system}"
#    optimized "${csfml-window}"
#	debug "${csfml-audio-d}"
#    debug "${csfml-network-d}"
#    debug "${csfml-graphics-d}"
#    debug "${csfml-system-d}"
#    debug "${csfml-window-d}"
#	"${GLEW_LIBRARIES}"
#	"${OPENGL_LIBRARIES}"
#	"${CMAKE_DL_LIBS}"
#	"${EXTRA_LIBRARIES}"
#	Threads::Threads
#	)
#
#target_include_directories(unnamedblocks PUBLIC
#	"${PROJECT_BINARY_DIR}"
#	"RHR"
#	"RHR/include"
#	"${GLEW_INCLUDE_DIRS}"
#	"${OPENGL_INCLUDE_DIRS}"
#	"${EXTRA_INCLUDE_DIRS}"
#	"${csfml_include}"
#	)
#
## Options
#if(CMAKE_SYSTEM_NAME STREQUAL "Windows")
#	# Default search in appdata
#	set(RES_FOLDER_SEARCH_PATH {APPDATA}/unnamedblocks CACHE STRING "(currently nop) Search dir for the res folder. {VARNAME} is environment variable. Blank for working directory")
#	set(MODS_FOLDER_SEARCH_PATH {APPDATA}/unnamedblocks CACHE STRING "(currently nop) Search dir for the mods folder. {VARNAME} is environment variable. Blank for working directory")
#elseif(UNIX AND NOT APPLE)
#	# Default search in .local/share in unix like oses
#	set(RES_FOLDER_SEARCH_PATH {HOME}/.local/share/unnamedblocks CACHE STRING "(currently nop) Search dir for the res folder. {VARNAME} is environment variable. Blank for working directory")
#	set(MODS_FOLDER_SEARCH_PATH {HOME}/.local/share/unnamedblocks CACHE STRING "(currently nop) Search dir for the mods folder. {VARNAME} is environment variable. Blank for working directory")
#else()
#	# Unknown platform, dont assume anything
#	set(RES_FOLDER_SEARCH_PATH "" CACHE STRING "(currently nop) Search dir for the res folder {VARNAME} is environment variable. Blank for working directory")
#	set(MODS_FOLDER_SEARCH_PATH "" CACHE STRING "(currently nop) Search dir for the mods folder {VARNAME} is environment variable. Blank for working directory")
#endif()
#
#set(VERSION_UNNAMED_BLOCKS "0" CACHE STRING "Version of Unnamed Blocks")
#option(IS_BETA "Wether or not Unnamed Blocks will be flagged as a beta build" ON)
#option(ENABLE_AVX2 "Enable AVX2 support. This is supported on Intel Haswell processors or newer and AMD Excavator processors and all Zen processors or newer" OFF)
#option(DISABLE_ALL_MOD_BUILDS "Disable building the set of vanilla mods." OFF)
#
#if (NOT DISABLE_ALL_MOD_BUILDS)
#	add_subdirectory(VanillaMods)
#endif()
#
## Copy res folder
add_custom_command(
	TARGET citiesbad POST_BUILD
	COMMAND ${CMAKE_COMMAND} -E copy_directory
	"${CMAKE_SOURCE_DIR}/res"
	$<TARGET_FILE_DIR:citiesbad>/res)
## Copy csfml
#add_custom_command(
#	TARGET unnamedblocks POST_BUILD
#	COMMAND ${CMAKE_COMMAND} -E copy_directory
#	"${CMAKE_SOURCE_DIR}/csfml"
#	$<TARGET_FILE_DIR:unnamedblocks>/csfml)
## Create mods folder for the mods to copy themselves into later
#add_custom_command(
#	TARGET unnamedblocks POST_BUILD
#	COMMAND ${CMAKE_COMMAND} -E make_directory
#	$<TARGET_FILE_DIR:unnamedblocks>/mods)
#add_custom_command(
#	TARGET unnamedblocks POST_BUILD
#	COMMAND ${CMAKE_COMMAND} -E copy_directory
#	"${CMAKE_SOURCE_DIR}/Cappuccino/include"
#	$<TARGET_FILE_DIR:unnamedblocks>/Cappuccino/include)
#add_custom_command(
#    TARGET unnamedblocks POST_BUILD
#    COMMAND ${CMAKE_COMMAND} -E make_directory
#    $<TARGET_FILE_DIR:unnamedblocks>/Cappuccino/lib/)
#add_custom_command(
#    TARGET unnamedblocks POST_BUILD
#    COMMAND ${CMAKE_COMMAND} -E copy
#    $<TARGET_FILE:Cappuccino>
#    $<TARGET_FILE_DIR:unnamedblocks>/Cappuccino/lib/)
## Copy dependencies on windows
#if(MSVC)
#	add_custom_command(
#		TARGET unnamedblocks POST_BUILD
#		COMMAND ${CMAKE_COMMAND} -E copy_directory
#		"${CMAKE_SOURCE_DIR}/tcc"
#		$<TARGET_FILE_DIR:unnamedblocks>)
#	add_custom_command(
#		TARGET unnamedblocks POST_BUILD
#		COMMAND ${CMAKE_COMMAND} -E copy
#		$<TARGET_FILE:sfml-system>
#		$<TARGET_FILE:sfml-window>
#		$<TARGET_FILE:sfml-graphics>
#		$<TARGET_FILE:sfml-network>
#		"${CMAKE_SOURCE_DIR}/cmake/bin/libtcc.dll"
#		$<TARGET_FILE_DIR:unnamedblocks>)
#	add_custom_command(
#		TARGET unnamedblocks POST_BUILD
#		COMMAND ${CMAKE_COMMAND} -E copy
#		$<TARGET_FILE:Cappuccino>
#		$<TARGET_FILE_DIR:unnamedblocks>/libCappuccino.dll)
#	add_custom_command(
#		TARGET unnamedblocks POST_BUILD
#		COMMAND ${CMAKE_COMMAND} -E copy_directory
#		"${CMAKE_SOURCE_DIR}/csfml/bin"
#		$<TARGET_FILE_DIR:unnamedblocks>)
#else()
#	add_custom_command(
#		TARGET unnamedblocks POST_BUILD
#		COMMAND ${CMAKE_COMMAND} -E copy
#		$<TARGET_FILE:Cappuccino>
#		$<TARGET_FILE_DIR:unnamedblocks>/libCappuccino.so)
#endif()
