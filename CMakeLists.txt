cmake_minimum_required(VERSION 3.0.0)
project(CMDComTool VERSION 0.1.0)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)
include(CTest)
enable_testing()
include_directories(ext/serial/include)
#LINK_DIRECTORIES(../serial/build/devel/lib)
add_executable(CMDComTool main.cpp)
target_include_directories(CMDComTool PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/include)

include(FetchContent)
FetchContent_Declare(serial
	GIT_REPOSITORY https://github.com/wjwwood/serial.git
	GIT_TAG 1.2.1
	SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/ext/serial
	)
FetchContent_MakeAvailable(serial)
target_link_libraries(CMDComTool serial)

set(CPACK_PROJECT_NAME ${PROJECT_NAME})
set(CPACK_PROJECT_VERSION ${PROJECT_VERSION})
include(CPack)
