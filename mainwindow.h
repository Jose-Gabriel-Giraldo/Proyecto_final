#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QStackedWidget>
#include "configuraciondificultad.h"

class MainWindow : public QMainWindow {
    Q_OBJECT
public:
    MainWindow(QWidget* parent = nullptr);

private:
    void crearMenu();
    void mostrar(QWidget* pantalla);
    void volverAlMenu();
    void error(const std::exception& e);

    void empezar(const ConfiguracionDificultad& dificultad);
    void cinematica1();
    void nivel1();
    void cinematica2();
    void nivel2();
    void epilogo();

    QStackedWidget* pantallas;
    QWidget* menu;
    QWidget* actual;
    ConfiguracionDificultad config;
    int puntajeNivel1, puntajeNivel2;
};

#endif
