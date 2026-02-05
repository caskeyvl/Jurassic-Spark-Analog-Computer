cmake_minimum_required(VERSION 3.22)

project(qt_hello VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CMAKE_AUTORCC ON)

find_package(Qt6 REQUIRED COMPONENTS Widgets QuickWidgets Qml Quick Charts)

qt_standard_project_setup()

qt_add_executable(qt_hello
	src/main.cpp
	src/mainwindow.h
	src/mainwindow.cpp
	src/mainwindow.ui
	resources.qrc
)

target_include_directories(qt_hello PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/src)

target_link_libraries(qt_hello PRIVATE 
	Qt6::Widgets
	Qt6::QuickWidgets
	Qt6::Qml
	Qt6::Quick
	Qt6::Charts
)
