# CMakeList.txt: проект CMake для rocket; включите исходный код и определения,
# укажите здесь логику для конкретного проекта.
#

# Добавьте источник в исполняемый файл этого проекта.

set(CMAKE_TOOLCHAIN_FILE "C:/vcpkg/scripts/buildsystems/vcpkg.cmake" CACHE STRING "")

add_executable (rocket "rocket.cpp" "rocket.h")

if (CMAKE_VERSION VERSION_GREATER 3.12)
  set_property(TARGET rocket PROPERTY CXX_STANDARD 20)
endif()

# TODO: Добавьте тесты и целевые объекты, если это необходимо.
find_package(glm CONFIG REQUIRED)
find_package(glfw3 CONFIG REQUIRED)
find_package(glad CONFIG REQUIRED)
find_package(tinyobjloader CONFIG REQUIRED)
target_link_libraries(rocket PRIVATE 
	glfw
	tinyobjloader::tinyobjloader 
	glad::glad 
	glm::glm
	opengl32
)
file(COPY ${CMAKE_CURRENT_SOURCE_DIR}/models DESTINATION ${CMAKE_CURRENT_BINARY_DIR})