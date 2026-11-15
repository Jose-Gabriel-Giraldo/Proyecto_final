#include <QApplication>
#include <iostream>
#include "vistaplantacion.h"

int main(int argc, char* argv[]) {
    QApplication app(argc, argv);
    try {
        VistaPlantacion vista;
        vista.setWindowTitle("Nivel 1 - La Plantación");
        vista.resize(1280, 720);
        QObject::connect(&vista, &VistaPlantacion::nivelTerminado, [&vista](bool gano, int puntaje) {
            std::cout << (gano ? "Gano" : "Perdio") << " con " << puntaje << " puntos" << std::endl;
            vista.reiniciar();
        });
        vista.show();
        vista.iniciar();
        return app.exec();
    } catch (const std::exception& e) {
        std::cout << "Error: " << e.what() << std::endl;
        return 1;
    }
}
