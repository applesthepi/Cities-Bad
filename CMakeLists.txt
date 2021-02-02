cmake_minimum_required(VERSION 3.10)

# use c++ 20
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED True)
set(CMAKE_POSITION_INDEPENDENT_CODE ON)
set(USE_FOLDERS ON)


project(citiesbad)

## Add X11 if using linux
## TODO: Proper wayland support
#if (CMAKE_SYSTEM_NAME STREQUAL "Linux")
#	set(LINUX True)
#	message(STATUS "Compiling For Linux")
#	find_package(X11 REQUIRED)
#	# Might need to go in UNIX, ill check later
#	find_package(TBB REQUIRED)
#	list(APPEND EXTRA_LIBRARIES ${X11_LIBRARIES})
#	list(APPEND EXTRA_INCLUDES ${X11_INCLUDE_DIRS})
#endif()
## Not really posix, just a term given to every os that follows some standards (so everything besides windows)
#if(UNIX)
#	set(POSIX True)
#endif()
set(CitiesBad_SRC
	# ==================================================================================
	# =============================================================== Headers
	# ==================================================================================
	
	# Root
	"${CMAKE_CURRENT_SOURCE_DIR}/CitiesBad/src/GameLayer.h"
	"${CMAKE_CURRENT_SOURCE_DIR}/CitiesBad/src/ui/Camera.h"
	
	# Road
	"${CMAKE_CURRENT_SOURCE_DIR}/CitiesBad/src/road/Clip.h"
	"${CMAKE_CURRENT_SOURCE_DIR}/CitiesBad/src/road/ClipGroup.h"
	"${CMAKE_CURRENT_SOURCE_DIR}/CitiesBad/src/road/Lane.h"
	"${CMAKE_CURRENT_SOURCE_DIR}/CitiesBad/src/road/Vertex.h"
	
	# Terrain
	"${CMAKE_CURRENT_SOURCE_DIR}/CitiesBad/src/terrain/Terrain.h"
	
	# Render
	"${CMAKE_CURRENT_SOURCE_DIR}/CitiesBad/src/render/Quad.h"
	
	# ==================================================================================
	# =============================================================== Source
	# ==================================================================================
	
	# Root
	"${CMAKE_CURRENT_SOURCE_DIR}/CitiesBad/src/main.cpp"
	"${CMAKE_CURRENT_SOURCE_DIR}/CitiesBad/src/GameLayer.cpp"
	"${CMAKE_CURRENT_SOURCE_DIR}/CitiesBad/src/ui/Camera.cpp"
	
	# Road
	"${CMAKE_CURRENT_SOURCE_DIR}/CitiesBad/src/road/Clip.cpp"
	"${CMAKE_CURRENT_SOURCE_DIR}/CitiesBad/src/road/ClipGroup.cpp"
	"${CMAKE_CURRENT_SOURCE_DIR}/CitiesBad/src/road/Lane.cpp"
	"${CMAKE_CURRENT_SOURCE_DIR}/CitiesBad/src/road/Vertex.cpp"
	
	# Terrain
	"${CMAKE_CURRENT_SOURCE_DIR}/CitiesBad/src/terrain/Terrain.cpp"
	
	# Render
	"${CMAKE_CURRENT_SOURCE_DIR}/CitiesBad/src/render/Quad.cpp"
	)

add_executable(citiesbad
	${CitiesBad_SRC}
	)
# Preserve folders in visual studio structure
source_group(TREE ${CMAKE_CURRENT_SOURCE_DIR}/CitiesBad/src FILES ${CitiesBad_SRC})
add_compile_definitions(NOISE_STATIC)
configure_file(config.h.in config.h)

set_property(TARGET citiesbad PROPERTY VS_DEBUGGER_WORKING_DIRECTORY "${CMAKE_BINARY_DIR}/Debug")
# Set unnamedblocks to the startup project
set_property(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} PROPERTY VS_STARTUP_PROJECT citiesbad)

add_subdirectory(OpenGLCore)
find_path(STB_INCLUDE_DIRS stb.h)
find_path(NOISE_INCLUDE_DIR NAMES noise/noise.h)

find_library(NOISE_LIBRARY NAMES noise-static)
find_library(NOISE_UTILS_LIBRARY NAMES noiseutils-static)

target_link_libraries(citiesbad PUBLIC
	OpenGLCore
	${NOISE_LIBRARY}
	${NOISE_UTILS_LIBRARY}
	)
	
target_include_directories(citiesbad PUBLIC
	"OpenGLCore"
	"OpenGLCore/include"
	"CitiesBad"
	"CitiesBad/src"
	${NOISE_INCLUDE_DIR}
	${STB_INCLUDE_DIRS}
	)

## Copy res folder
add_custom_command(
	TARGET citiesbad POST_BUILD
	COMMAND ${CMAKE_COMMAND} -E copy_directory
	"${CMAKE_SOURCE_DIR}/res"
	$<TARGET_FILE_DIR:citiesbad>/res)
