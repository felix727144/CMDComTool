cmake_minimum_required(VERSION 3.0.0)
project(CMDComTool VERSION 0.1.0)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)

if(APPLE)
	find_library(IOKIT_LIBRARY IOKit)
	find_library(FOUNDATION_LIBRARY Foundation)
endif()

aux_source_directory("./source"                     SOURCE_CPP0)
aux_source_directory("./serial_src/src"                 SOURCE_CPP1)
aux_source_directory("./serial_src/src/impl"            SOURCE_CPP2)
aux_source_directory("./serial_src/src/impl/list_ports" SOURCE_CPP3)
aux_source_directory("./ftxui/src/ftxui/component"  SOURCE_CPP4)
aux_source_directory("./ftxui/src/ftxui/dom"        SOURCE_CPP5)
aux_source_directory("./ftxui/src/ftxui/screen"     SOURCE_CPP6)

set(SOURCE_MAIN ${SOURCE_CPP0})
set(SOURCE_WJWOOOD_SERIAL ${SOURCE_CPP1} ${SOURCE_CPP2} ${SOURCE_CPP3})
set(SOURCE_FTXUI ${SOURCE_CPP4} ${SOURCE_CPP5} ${SOURCE_CPP6})

include_directories("serial_src/include")
include_directories("ftxui/include")
include_directories("ftxui/src")

add_executable(${PROJECT_NAME} ${SOURCE_WJWOOOD_SERIAL} ${SOURCE_FTXUI} ${SOURCE_MAIN})

if(APPLE)
	target_link_libraries(${PROJECT_NAME} ${FOUNDATION_LIBRARY} ${IOKIT_LIBRARY})
elseif(UNIX)
    target_link_libraries(${PROJECT_NAME} pthread)
endif()

#install(TARGETS ${PROJECT_NAME} DESTINATION bin)
