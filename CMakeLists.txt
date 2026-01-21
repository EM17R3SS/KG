cmake_minimum_required(VERSION 3.16)
project(BezierCurve3D)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(Qt6 REQUIRED COMPONENTS Core Widgets OpenGLWidgets)

qt_standard_project_setup()

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTORCC ON)

add_executable(BezierCurve3D
    main.cpp
    mainwindow.cpp
    glwidget.cpp
    pointtablewidget.cpp
)

target_link_libraries(BezierCurve3D
    Qt6::Core
    Qt6::Widgets
    Qt6::OpenGLWidgets
)

if(QT_VERSION_MAJOR EQUAL 6)
    qt_import_plugins(BezierCurve3D)
endif()
