cmake_minimum_required(VERSION 3.16)

project(proyecto_final VERSION 0.1 LANGUAGES CXX)

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(QT NAMES Qt6 Qt5 REQUIRED COMPONENTS Widgets)
find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS Widgets)

set(PROJECT_SOURCES
        main.cpp
        mainwindow.cpp
        mainwindow.h
        mainwindow.ui
)

if(${QT_VERSION_MAJOR} GREATER_EQUAL 6)
    qt_add_executable(proyecto_final
        MANUAL_FINALIZATION
        ${PROJECT_SOURCES}
        tipo.h
        entidad.h entidad.cpp
        ModeloFisico.h
        fisicaparabolica.h fisicaparabolica.cpp
        fisicapendular.h fisicapendular.cpp
        fisicaarrastre.h fisicaarrastre.cpp
        personaje.h personaje.cpp
        proyectil.h proyectil.cpp
        morty.h morty.cpp
        rick.h rick.cpp
        bananamutante.h bananamutante.cpp
        configuraciondificultad.h configuraciondificultad.cpp
    )
else()
    if(ANDROID)
        add_library(proyecto_final SHARED
            ${PROJECT_SOURCES}
        )
    else()
        add_executable(proyecto_final
            ${PROJECT_SOURCES}
        )
    endif()
endif()

target_link_libraries(proyecto_final PRIVATE Qt${QT_VERSION_MAJOR}::Widgets)

if(${QT_VERSION} VERSION_LESS 6.1.0)
  set(BUNDLE_ID_OPTION MACOSX_BUNDLE_GUI_IDENTIFIER com.example.proyecto_final)
endif()
set_target_properties(proyecto_final PROPERTIES
    ${BUNDLE_ID_OPTION}
    MACOSX_BUNDLE_BUNDLE_VERSION ${PROJECT_VERSION}
    MACOSX_BUNDLE_SHORT_VERSION_STRING ${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR}
    MACOSX_BUNDLE TRUE
    WIN32_EXECUTABLE FALSE
)

include(GNUInstallDirs)
install(TARGETS proyecto_final
    BUNDLE DESTINATION .
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)

if(QT_VERSION_MAJOR EQUAL 6)
    qt_finalize_executable(proyecto_final)
endif()
