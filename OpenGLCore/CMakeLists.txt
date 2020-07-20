cmake_minimum_required(VERSION 3.10)

project(OpenGLCore)
configure_file(config.h.in config.h)
set(OpenGLCore_SRC
	# ==================================================================================
	# =============================================================== Headers
	# ==================================================================================
	
	# Root
	"${CMAKE_CURRENT_SOURCE_DIR}/include/GLCore.h"
	"${CMAKE_CURRENT_SOURCE_DIR}/include/GLCoreUtils.h"
	"${CMAKE_CURRENT_SOURCE_DIR}/include/glpch.h"
	
	# Core
	"${CMAKE_CURRENT_SOURCE_DIR}/include/GLCore/Core/Application.h"
	"${CMAKE_CURRENT_SOURCE_DIR}/include/GLCore/Core/Core.h"
	"${CMAKE_CURRENT_SOURCE_DIR}/include/GLCore/Core/Input.h"
	"${CMAKE_CURRENT_SOURCE_DIR}/include/GLCore/Core/KeyCodes.h"
	"${CMAKE_CURRENT_SOURCE_DIR}/include/GLCore/Core/Layer.h"
	"${CMAKE_CURRENT_SOURCE_DIR}/include/GLCore/Core/LayerStack.h"
	"${CMAKE_CURRENT_SOURCE_DIR}/include/GLCore/Core/Log.h"
	"${CMAKE_CURRENT_SOURCE_DIR}/include/GLCore/Core/MouseButtonCodes.h"
	"${CMAKE_CURRENT_SOURCE_DIR}/include/GLCore/Core/Timestep.h"
	"${CMAKE_CURRENT_SOURCE_DIR}/include/GLCore/Core/Window.h"
	
	# Events
	"${CMAKE_CURRENT_SOURCE_DIR}/include/GLCore/Events/ApplicationEvent.h"
	"${CMAKE_CURRENT_SOURCE_DIR}/include/GLCore/Events/Event.h"
	"${CMAKE_CURRENT_SOURCE_DIR}/include/GLCore/Events/KeyEvent.h"
	"${CMAKE_CURRENT_SOURCE_DIR}/include/GLCore/Events/MouseEvent.h"
	
	# ImGui
	"${CMAKE_CURRENT_SOURCE_DIR}/include/GLCore/ImGui/ImGuiLayer.h"
	"${CMAKE_CURRENT_SOURCE_DIR}/include/GLCore/ImGui/imgui_impl_opengl3.h"
	"${CMAKE_CURRENT_SOURCE_DIR}/include/GLCore/ImGui/imgui_impl_glfw.h"
	
	# Util
	"${CMAKE_CURRENT_SOURCE_DIR}/include/GLCore/Util/OpenGLDebug.h"
	"${CMAKE_CURRENT_SOURCE_DIR}/include/GLCore/Util/Shader.h"
	# ==================================================================================
	# =============================================================== Source
	# ==================================================================================
	
	# Root
	"${CMAKE_CURRENT_SOURCE_DIR}/include/glpch.cpp"
	
	# Core
	"${CMAKE_CURRENT_SOURCE_DIR}/include/GLCore/Core/Application.cpp"
	"${CMAKE_CURRENT_SOURCE_DIR}/include/GLCore/Core/Layer.cpp"
	"${CMAKE_CURRENT_SOURCE_DIR}/include/GLCore/Core/LayerStack.cpp"
	"${CMAKE_CURRENT_SOURCE_DIR}/include/GLCore/Core/Log.cpp"
	
	# ImGui
	"${CMAKE_CURRENT_SOURCE_DIR}/include/GLCore/ImGui/ImGuiBuild.cpp"
	"${CMAKE_CURRENT_SOURCE_DIR}/include/GLCore/ImGui/ImGuiLayer.cpp"
	"${CMAKE_CURRENT_SOURCE_DIR}/include/GLCore/ImGui/imgui_impl_opengl3.cpp"
	"${CMAKE_CURRENT_SOURCE_DIR}/include/GLCore/ImGui/imgui_impl_glfw.cpp"
	
	# Util
	"${CMAKE_CURRENT_SOURCE_DIR}/include/GLCore/Util/OpenGLDebug.cpp"
	"${CMAKE_CURRENT_SOURCE_DIR}/include/GLCore/Util/Shader.cpp"
	)
		
find_package(glad CONFIG REQUIRED)
find_package(glfw3 CONFIG REQUIRED)
find_package(glm CONFIG REQUIRED)
find_package(imgui CONFIG REQUIRED)
find_package(spdlog CONFIG REQUIRED)

add_library(OpenGLCore STATIC
	${OpenGLCore_SRC}
)

target_link_libraries(OpenGLCore PUBLIC
	glad
	glfw
	glm
	imgui
	spdlog
)

target_include_directories(OpenGLCore PUBLIC
	"${CMAKE_CURRENT_SOURCE_DIR}/include"
)

target_precompile_headers(OpenGLCore PUBLIC
	"${CMAKE_CURRENT_SOURCE_DIR}/include/glpch.h"
)

#find_path(CATCH_INCLUDE_DIR catch.hpp)
#include_directories(${CATCH_INCLUDE_DIR})

#target_include_directories(OpenGLCore PUBLIC
#	glad
#)
#target_link_libraries(RHR PUBLIC
#	Espresso
#	Cappuccino
#	sfml-graphics
#	sfml-system
#	sfml-window
#	sfml-network
#	${libtcc}
#)
#target_include_directories(RHR PUBLIC
#	"${PROJECT_BINARY_DIR}"
#	"${CMAKE_CURRENT_SOURCE_DIR}"
#	"${CMAKE_CURRENT_SOURCE_DIR}/include"
#	"${CMAKE_CURRENT_SOURCE_DIR}/include/RHR"
#	"${CMAKE_SOURCE_DIR}/Espresso/include"
#	${tcc_include}
#)
#
source_group(TREE ${CMAKE_CURRENT_SOURCE_DIR}/include FILES ${OpenGLCore_SRC})
