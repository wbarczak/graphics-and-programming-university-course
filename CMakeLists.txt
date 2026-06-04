cmake_minimum_required(VERSION 3.28)
project(Platformer VERSION 0.1.0)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

include(FetchContent)
FetchContent_Declare(
	SFML
	GIT_REPOSITORY https://github.com/SFML/SFML.git
	GIT_TAG 3.0.2
	GIT_SHALLOW ON
	EXCLUDE_FROM_ALL
	SYSTEM
)
FetchContent_MakeAvailable(SFML)

FetchContent_Declare(
	imgui
	GIT_REPOSITORY https://github.com/ocornut/imgui.git
	GIT_TAG v1.91.1
	GIT_SHALLOW ON
	EXCLUDE_FROM_ALL
	SYSTEM
)
FetchContent_MakeAvailable(imgui)

set(IMGUI_DIR "${imgui_SOURCE_DIR}" CACHE PATH "imgui top-level directory" FORCE)
set(IMGUI_SFML_FIND_SFML OFF CACHE BOOL "" FORCE)

FetchContent_Declare(
	imgui_sfml
	GIT_REPOSITORY https://github.com/SFML/imgui-sfml.git
	GIT_TAG v3.0
	GIT_SHALLOW ON
	EXCLUDE_FROM_ALL
	SYSTEM
)
FetchContent_MakeAvailable(imgui_sfml)

FetchContent_Declare(
    nlohmann_json
    GIT_REPOSITORY https://github.com/nlohmann/json.git
    GIT_TAG v3.11.2
	GIT_SHALLOW ON
)
FetchContent_MakeAvailable(nlohmann_json)

FetchContent_Declare(
	entt
	GIT_REPOSITORY https://github.com/skypjack/entt
	GIT_TAG v3.16.0
	GIT_SHALLOW ON
)
FetchContent_MakeAvailable(entt)

FetchContent_Declare(
	spdlog
	GIT_REPOSITORY https://github.com/gabime/spdlog
	GIT_TAG v1.17.0
	GIT_SHALLOW ON
)
FetchContent_MakeAvailable(spdlog)

FetchContent_Declare(
	tgui
	GIT_REPOSITORY https://github.com/texus/TGUI
	GIT_TAG v1.12.1
	GIT_SHALLOW ON
)
FetchContent_MakeAvailable(tgui)

set(SFML_GRAPHICS)

file(GLOB_RECURSE SOURCES "${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp")

add_executable(${PROJECT_NAME} ${SOURCES}
	#WIN32
)

target_include_directories(${PROJECT_NAME} PRIVATE
	"${CMAKE_CURRENT_SOURCE_DIR}/src"
	#${nlohmann_json_SOURCE_DIR}/include
)

target_link_libraries(${PROJECT_NAME} PRIVATE
	SFML::Graphics
	SFML::System
	SFML::Audio
	ImGui-SFML::ImGui-SFML
	#SFML::Main
	nlohmann_json
	EnTT::EnTT
	spdlog::spdlog_header_only
	TGUI::TGUI
)

file(COPY ${CMAKE_SOURCE_DIR}/resources/ DESTINATION ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/resources)

add_definitions(-DRESOURCES="${CMAKE_SOURCE_DIR}/resources/")

set_property(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} PROPERTY VS_STARTUP_PROJECT ${PROJECT_NAME})