cmake_minimum_required(VERSION 3.0.0)
project(CMDComTool VERSION 0.1.0)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)
include(CTest)
enable_testing()
include_directories(serial/include)
#LINK_DIRECTORIES(../serial/build/devel/lib)
add_subdirectory(serial)
add_executable(CMDComTool main.cpp)
target_include_directories(CMDComTool PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/include)

target_link_libraries(CMDComTool serial)

install(TARGETS CMDComTool
	DESTINATION bin)
set(CPACK_PROJECT_NAME ${PROJECT_NAME})
set(CPACK_PROJECT_VERSION ${PROJECT_VERSION})
include(CPack)
