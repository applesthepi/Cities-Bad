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
	"${CMAKE_CURRENT_SOURCE_DIR}/include/GLCore/ImGui/imgui_impl_glfw.h"
	"${CMAKE_CURRENT_SOURCE_DIR}/include/GLCore/ImGui/ImGuiLayer.h"
	"${CMAKE_CURRENT_SOURCE_DIR}/include/GLCore/ImGui/imstb_rectpack.h"
	"${CMAKE_CURRENT_SOURCE_DIR}/include/GLCore/ImGui/imstb_truetype.h"
	"${CMAKE_CURRENT_SOURCE_DIR}/include/GLCore/ImGui/imgui_internal.h"
	"${CMAKE_CURRENT_SOURCE_DIR}/include/GLCore/ImGui/imstb_textedit.h"
	#"${CMAKE_CURRENT_SOURCE_DIR}/include/GLCore/ImGui/misc/freetype/imgui_freetype.h"
	#"${CMAKE_CURRENT_SOURCE_DIR}/include/GLCore/ImGui/misc/cpp/imgui_stdlib.h"
	"${CMAKE_CURRENT_SOURCE_DIR}/include/GLCore/ImGui/imgui_impl_opengl3.h"
	"${CMAKE_CURRENT_SOURCE_DIR}/include/GLCore/ImGui/imgui.h"
	"${CMAKE_CURRENT_SOURCE_DIR}/include/GLCore/ImGui/imconfig.h"
	
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
	#"${CMAKE_CURRENT_SOURCE_DIR}/include/GLCore/ImGui/misc/cpp/imgui_stdlib.cpp"
	#"${CMAKE_CURRENT_SOURCE_DIR}/include/GLCore/ImGui/misc/fonts/binary_to_compressed_c.cpp"
	#"${CMAKE_CURRENT_SOURCE_DIR}/include/GLCore/ImGui/misc/freetype/imgui_freetype.cpp"
	"${CMAKE_CURRENT_SOURCE_DIR}/include/GLCore/ImGui/imgui_demo.cpp"
	"${CMAKE_CURRENT_SOURCE_DIR}/include/GLCore/ImGui/imgui_draw.cpp"
	"${CMAKE_CURRENT_SOURCE_DIR}/include/GLCore/ImGui/imgui.cpp"
	"${CMAKE_CURRENT_SOURCE_DIR}/include/GLCore/ImGui/imgui_widgets.cpp"
	"${CMAKE_CURRENT_SOURCE_DIR}/include/GLCore/ImGui/ImGuiLayer.cpp"
	"${CMAKE_CURRENT_SOURCE_DIR}/include/GLCore/ImGui/ImGuiBuild.cpp"
	# Util
	"${CMAKE_CURRENT_SOURCE_DIR}/include/GLCore/Util/OpenGLDebug.cpp"
	"${CMAKE_CURRENT_SOURCE_DIR}/include/GLCore/Util/Shader.cpp"

	"${CMAKE_CURRENT_SOURCE_DIR}/include/Platform/Windows/WindowsWindow.h"
	"${CMAKE_CURRENT_SOURCE_DIR}/include/Platform/Windows/WindowsInput.cpp"
	"${CMAKE_CURRENT_SOURCE_DIR}/include/Platform/Windows/WindowsWindow.cpp"
	"${CMAKE_CURRENT_SOURCE_DIR}/include/Platform/Windows/WindowsInput.h"
	)
		
find_package(glad CONFIG REQUIRED)
find_package(glfw3 CONFIG REQUIRED)
find_package(glm CONFIG REQUIRED)
find_package(spdlog CONFIG REQUIRED)


add_library(OpenGLCore STATIC
	${OpenGLCore_SRC}
)

target_link_libraries(OpenGLCore PUBLIC
	glad::glad
	glfw
	spdlog::spdlog spdlog::spdlog_header_only
	glm
)

target_include_directories(OpenGLCore PUBLIC
	"${CMAKE_CURRENT_SOURCE_DIR}/include"
)

target_precompile_headers(OpenGLCore PUBLIC
	"${CMAKE_CURRENT_SOURCE_DIR}/include/glpch.h"
)

source_group(TREE ${CMAKE_CURRENT_SOURCE_DIR}/include FILES ${OpenGLCore_SRC})
