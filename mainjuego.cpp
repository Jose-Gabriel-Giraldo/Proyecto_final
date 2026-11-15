#include "mainwindow.h"
#include <QApplication>
#include <QMessageBox>
#include <stdexcept>

// Punto de entrada del juego con interfaz grafica.
// (main.cpp se conserva como programa de pruebas de la logica por consola.)
int main(int argc, char *argv[])
{
    QApplication app(argc, argv);
    try {
        MainWindow ventana;
        ventana.show();
        return app.exec();
    } catch (const std::exception& e) {
        // Por ejemplo: falta un sprite en recursos.qrc
        QMessageBox::critical(nullptr, "Error al iniciar el juego", e.what());
        return 1;
    }
}
