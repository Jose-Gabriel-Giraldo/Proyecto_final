cmake_minimum_required(VERSION 3.16)

project(proyecto_final VERSION 0.1 LANGUAGES CXX)

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(QT NAMES Qt6 Qt5 REQUIRED COMPONENTS Widgets)
find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS Widgets)
find_package(Qt${QT_VERSION_MAJOR} QUIET COMPONENTS Multimedia)

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
        modelofisico.h
        fisicaparabolica.h fisicaparabolica.cpp
        fisicapendular.h fisicapendular.cpp
        fisicaarrastre.h fisicaarrastre.cpp
        personaje.h personaje.cpp
        proyectil.h proyectil.cpp
        morty.h morty.cpp
        rick.h rick.cpp
        bananamutante.h bananamutante.cpp
        configuraciondificultad.h configuraciondificultad.cpp
        moduloaprendizaje.h moduloaprendizaje.cpp
        modulopercepcion.h modulopercepcion.cpp
        modulorazonamiento.h modulorazonamiento.cpp
        moduloaccion.h moduloaccion.cpp
        palmerajefe.h palmerajefe.cpp
        nivel.h nivel.cpp
        nivelplantacion.h nivelplantacion.cpp
        niveljefe.h niveljefe.cpp
        animacion.h animacion.cpp
        vistaplantacion.h vistaplantacion.cpp
        vistajefe.h vistajefe.cpp
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

file(GLOB RECURSOS RELATIVE ${CMAKE_CURRENT_SOURCE_DIR} CONFIGURE_DEPENDS recursos/*.png recursos/*.txt)
qt_add_resources(proyecto_final "recursos" PREFIX "/" BASE recursos FILES ${RECURSOS})

target_link_libraries(proyecto_final PRIVATE Qt${QT_VERSION_MAJOR}::Widgets)

if(TARGET Qt${QT_VERSION_MAJOR}::Multimedia AND EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/recursos/musica.mp3)
    qt_add_resources(proyecto_final "musica" PREFIX "/" BASE recursos BIG_RESOURCES FILES recursos/musica.mp3)
    target_link_libraries(proyecto_final PRIVATE Qt${QT_VERSION_MAJOR}::Multimedia)
    target_compile_definitions(proyecto_final PRIVATE CON_MUSICA)
    message(STATUS "Musica activada")
else()
    message(STATUS "Sin Qt Multimedia o sin recursos/musica.mp3: el nivel 2 corre sin musica")
endif()

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
