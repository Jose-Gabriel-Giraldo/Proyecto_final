#include <QApplication>
#include <iostream>
#include "configuraciondificultad.h"
#include "vistajefe.h"

int main(int argc, char* argv[]) {
    QApplication app(argc, argv);
    try {
        VistaJefe vista(ConfiguracionDificultad::normal());
        vista.setWindowTitle("Nivel 2 - La Palmera Madre");
        vista.resize(1280, 720);
        QObject::connect(&vista, &VistaJefe::nivelTerminado, [&vista](bool gano, int puntaje) {
            std::cout << (gano ? "Gano" : "Perdio") << " con " << puntaje << " puntos" << std::endl;
            vista.reiniciar();
            vista.iniciar();
        });
        vista.show();
        vista.iniciar();
        return app.exec();
    } catch (const std::exception& e) {
        std::cout << "Error: " << e.what() << std::endl;
        return 1;
    }
}
