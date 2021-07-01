# CMakeList.txt : Top-level CMake project file, do global configuration
# and include sub-projects here.
#
cmake_minimum_required (VERSION 3.16)

project ("learn-gl" VERSION 1.0.0)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED OFF)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_UNITY_BUILD ON)

# Detect platform
#WINDOWS   	=  Windows
#ANDROID    =  Android
#LINUX      =  Linux
#MACOS    	=  macOS
#IOS    	=  iOS
if(${CMAKE_SYSTEM_NAME} MATCHES "Windows")
	set(WINDOWS TRUE)
	set(PLATFORM_FOLDER windows)
elseif(${CMAKE_SYSTEM_NAME} MATCHES "Android")
	set(ANDROID TRUE)
	set(PLATFORM_FOLDER android)
elseif(${CMAKE_SYSTEM_NAME} MATCHES "Linux")
	if(ANDROID)
		set(ANDROID TRUE)
		set(PLATFORM_FOLDER android)
	else()
		set(LINUX TRUE)
		set(PLATFORM_FOLDER linux)
	endif()
elseif(${CMAKE_SYSTEM_NAME} MATCHES "Darwin")
	set(APPLE TRUE)
	set(MACOS TRUE)
	set(PLATFORM_FOLDER macos)
elseif(${CMAKE_SYSTEM_NAME} MATCHES "iOS")
	set(APPLE TRUE)
	set(IOS TRUE)
	set(PLATFORM_FOLDER ios)
else()
	message(FATAL_ERROR "Unsupported platform, CMake will exit")
	return()
endif()

# 3rdparty include
include_directories (
	${CMAKE_SOURCE_DIR}/3rdparty/glad-0.1.34/include
	${CMAKE_SOURCE_DIR}/3rdparty/glfw-3.3.2/include
	${CMAKE_SOURCE_DIR}/3rdparty/spirv/include
)

# 3rdparty sources
file(GLOB_RECURSE 3RDPARTY_SOURCE_FILES
	${CMAKE_SOURCE_DIR}/3rdparty/*.c
	${CMAKE_SOURCE_DIR}/3rdparty/*.cc
	${CMAKE_SOURCE_DIR}/3rdparty/*.cpp
)

# 3rdparty libs
link_directories(
	${CMAKE_SOURCE_DIR}/3rdparty/glfw-3.3.2/lib/${PLATFORM_FOLDER}
	${CMAKE_SOURCE_DIR}/3rdparty/spirv/lib/${PLATFORM_FOLDER}
)

# 3rdparty link
link_libraries(glfw3)
#link_libraries(glfw3 spirv-cross-core spirv-cross-glsl)

if (APPLE)
	link_libraries(
		"-framework AppKit"
		"-framework IOKit"
		"-framework CoreGraphics"
	)
endif()

# Projects
include_directories(
	 ${CMAKE_SOURCE_DIR}/include
)

set(SOURCE_FILES
	${CMAKE_CURRENT_SOURCE_DIR}/src/nanojpeg.c
	${CMAKE_CURRENT_SOURCE_DIR}/src/entry.cpp
)

# List projects
add_executable(${PROJECT_NAME}_01 ${3RDPARTY_SOURCE_FILES} ${SOURCE_FILES} ${CMAKE_SOURCE_DIR}/src/learn_gl_01.cpp)
add_executable(${PROJECT_NAME}_02 ${3RDPARTY_SOURCE_FILES} ${CMAKE_SOURCE_DIR}/src/learn_gl_02.cpp)
add_executable(${PROJECT_NAME}_03 ${3RDPARTY_SOURCE_FILES} ${SOURCE_FILES} ${CMAKE_SOURCE_DIR}/src/learn_gl_03.cpp)
add_executable(${PROJECT_NAME}_04 ${3RDPARTY_SOURCE_FILES} ${SOURCE_FILES} ${CMAKE_SOURCE_DIR}/src/learn_gl_04.cpp)
add_executable(${PROJECT_NAME}_05 ${3RDPARTY_SOURCE_FILES} ${SOURCE_FILES} ${CMAKE_SOURCE_DIR}/src/learn_gl_05.cpp)
add_executable(${PROJECT_NAME}_06 ${3RDPARTY_SOURCE_FILES} ${SOURCE_FILES} ${CMAKE_SOURCE_DIR}/src/learn_gl_06.cpp)
add_executable(${PROJECT_NAME}_07 ${3RDPARTY_SOURCE_FILES} ${SOURCE_FILES} ${CMAKE_SOURCE_DIR}/src/learn_gl_07.cpp)
add_executable(${PROJECT_NAME}_08 ${3RDPARTY_SOURCE_FILES} ${SOURCE_FILES} ${CMAKE_SOURCE_DIR}/src/learn_gl_08.cpp)
add_executable(${PROJECT_NAME}_09 ${3RDPARTY_SOURCE_FILES} ${SOURCE_FILES} ${CMAKE_SOURCE_DIR}/src/learn_gl_09.cpp)
add_executable(${PROJECT_NAME}_10 ${3RDPARTY_SOURCE_FILES} ${SOURCE_FILES} ${CMAKE_SOURCE_DIR}/src/learn_gl_10.cpp)
add_executable(${PROJECT_NAME}_11 ${3RDPARTY_SOURCE_FILES} ${SOURCE_FILES} ${CMAKE_SOURCE_DIR}/src/learn_gl_11.cpp)
add_executable(${PROJECT_NAME}_12 ${3RDPARTY_SOURCE_FILES} ${SOURCE_FILES} ${CMAKE_SOURCE_DIR}/src/learn_gl_12.cpp)
add_executable(${PROJECT_NAME}_13 ${3RDPARTY_SOURCE_FILES} ${SOURCE_FILES} ${CMAKE_SOURCE_DIR}/src/learn_gl_13.cpp)
add_executable(${PROJECT_NAME}_14 ${3RDPARTY_SOURCE_FILES} ${SOURCE_FILES} ${CMAKE_SOURCE_DIR}/src/learn_gl_14.cpp)
add_executable(${PROJECT_NAME}_15 ${3RDPARTY_SOURCE_FILES} ${SOURCE_FILES} ${CMAKE_SOURCE_DIR}/src/learn_gl_15.cpp)
add_executable(${PROJECT_NAME}_16 ${3RDPARTY_SOURCE_FILES} ${SOURCE_FILES} ${CMAKE_SOURCE_DIR}/src/learn_gl_16.cpp)

add_executable(example_01 ${3RDPARTY_SOURCE_FILES} ${SOURCE_FILES} ${CMAKE_SOURCE_DIR}/src/example_01.cpp)
add_executable(example_02 ${3RDPARTY_SOURCE_FILES} ${SOURCE_FILES} ${CMAKE_SOURCE_DIR}/src/example_02.cpp)
add_executable(example_03 ${3RDPARTY_SOURCE_FILES} ${SOURCE_FILES} ${CMAKE_SOURCE_DIR}/src/example_03.cpp)
add_executable(example_04 ${3RDPARTY_SOURCE_FILES} ${SOURCE_FILES} ${CMAKE_SOURCE_DIR}/src/example_04.cpp)
add_executable(example_05 ${3RDPARTY_SOURCE_FILES} ${SOURCE_FILES} ${CMAKE_SOURCE_DIR}/src/example_05.cpp)
add_executable(example_06 ${3RDPARTY_SOURCE_FILES} ${SOURCE_FILES} ${CMAKE_SOURCE_DIR}/src/example_06.cpp)
add_executable(example_07 ${3RDPARTY_SOURCE_FILES} ${SOURCE_FILES} ${CMAKE_SOURCE_DIR}/src/example_07.cpp)
add_executable(example_08 ${3RDPARTY_SOURCE_FILES} ${SOURCE_FILES} ${CMAKE_SOURCE_DIR}/src/example_08.cpp)
add_executable(example_09 ${3RDPARTY_SOURCE_FILES} ${SOURCE_FILES} ${CMAKE_SOURCE_DIR}/src/example_09.cpp)
add_executable(example_10 ${3RDPARTY_SOURCE_FILES} ${SOURCE_FILES} ${CMAKE_SOURCE_DIR}/src/example_10.cpp)
add_executable(example_11 ${3RDPARTY_SOURCE_FILES} ${SOURCE_FILES} ${CMAKE_SOURCE_DIR}/src/example_11.cpp)
add_executable(example_12 ${3RDPARTY_SOURCE_FILES} ${SOURCE_FILES} ${CMAKE_SOURCE_DIR}/src/example_12.cpp)
add_executable(example_13 ${3RDPARTY_SOURCE_FILES} ${SOURCE_FILES} ${CMAKE_SOURCE_DIR}/src/example_13.cpp)
add_executable(example_14 ${3RDPARTY_SOURCE_FILES} ${SOURCE_FILES} ${CMAKE_SOURCE_DIR}/src/example_14.cpp)
add_executable(example_15 ${3RDPARTY_SOURCE_FILES} ${SOURCE_FILES} ${CMAKE_SOURCE_DIR}/src/example_15.cpp)
add_executable(example_16 ${3RDPARTY_SOURCE_FILES} ${SOURCE_FILES} ${CMAKE_SOURCE_DIR}/src/example_16.cpp)
add_executable(example_17 ${3RDPARTY_SOURCE_FILES} ${SOURCE_FILES} ${CMAKE_SOURCE_DIR}/src/example_17.cpp)
add_executable(example_18 ${3RDPARTY_SOURCE_FILES} ${SOURCE_FILES} ${CMAKE_SOURCE_DIR}/src/example_18.cpp)
add_executable(example_19 ${3RDPARTY_SOURCE_FILES} ${SOURCE_FILES} ${CMAKE_SOURCE_DIR}/src/example_19.cpp)

# Data
if (EXISTS ${CMAKE_SOURCE_DIR}/data)
add_custom_command(TARGET  ${PROJECT_NAME}_08 PRE_BUILD
				   COMMAND ${CMAKE_COMMAND} -E copy_directory
				   ${CMAKE_SOURCE_DIR}/data $<TARGET_FILE_DIR:${PROJECT_NAME}_08>/data
)
add_custom_command(TARGET  example_05 PRE_BUILD
				   COMMAND ${CMAKE_COMMAND} -E copy_directory
				   ${CMAKE_SOURCE_DIR}/data $<TARGET_FILE_DIR:example_05>/data
)
add_custom_command(TARGET  example_09 PRE_BUILD
				   COMMAND ${CMAKE_COMMAND} -E copy_directory
				   ${CMAKE_SOURCE_DIR}/data $<TARGET_FILE_DIR:example_09>/data
)
add_custom_command(TARGET  example_10 PRE_BUILD
				   COMMAND ${CMAKE_COMMAND} -E copy_directory
				   ${CMAKE_SOURCE_DIR}/data $<TARGET_FILE_DIR:example_10>/data
)
add_custom_command(TARGET  example_11 PRE_BUILD
				   COMMAND ${CMAKE_COMMAND} -E copy_directory
				   ${CMAKE_SOURCE_DIR}/data $<TARGET_FILE_DIR:example_11>/data
)
add_custom_command(TARGET  example_12 PRE_BUILD
				   COMMAND ${CMAKE_COMMAND} -E copy_directory
				   ${CMAKE_SOURCE_DIR}/data $<TARGET_FILE_DIR:example_12>/data
)
add_custom_command(TARGET  example_13 PRE_BUILD
				   COMMAND ${CMAKE_COMMAND} -E copy_directory
				   ${CMAKE_SOURCE_DIR}/data $<TARGET_FILE_DIR:example_13>/data
)
add_custom_command(TARGET  example_14 PRE_BUILD
				   COMMAND ${CMAKE_COMMAND} -E copy_directory
				   ${CMAKE_SOURCE_DIR}/data $<TARGET_FILE_DIR:example_14>/data
)
add_custom_command(TARGET  example_15 PRE_BUILD
				   COMMAND ${CMAKE_COMMAND} -E copy_directory
				   ${CMAKE_SOURCE_DIR}/data $<TARGET_FILE_DIR:example_15>/data
)
add_custom_command(TARGET  example_16 PRE_BUILD
				   COMMAND ${CMAKE_COMMAND} -E copy_directory
				   ${CMAKE_SOURCE_DIR}/data $<TARGET_FILE_DIR:example_16>/data
)
add_custom_command(TARGET  example_17 PRE_BUILD
				   COMMAND ${CMAKE_COMMAND} -E copy_directory
				   ${CMAKE_SOURCE_DIR}/data $<TARGET_FILE_DIR:example_17>/data
)
endif()