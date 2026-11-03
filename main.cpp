#include "entidad.h"
#include "fisicaparabolica.h"
#include <cmath>
#include <iostream>
#include <stdexcept>

class CuerpoPrueba : public Entidad {
public:
    CuerpoPrueba(double x, double y, ModeloFisico* f) : Entidad(x, y, 10, 10, f) {}
    void actualizar(double dt) override { fisica->aplicar(*this, dt); }
};

int main() {
    const double dt = 1.0 / 600;

    CuerpoPrueba salto(0, 0, new FisicaParabolica(1400));
    salto.setVelocidad(200, -750);
    for (int i = 0; i < 300; i++) salto.actualizar(dt);
    std::cout << "Despues de 0.5 s: x = " << salto.getX() << " (esperado 100), y = " << salto.getY()
              << " (esperado cerca de -200)" << std::endl;

    double vx, vy;
    FisicaParabolica::velocidadApuntado(400 - 1030, 580 - 330, 1.0, 1400, vx, vy);
    CuerpoPrueba racimo(1030, 330, new FisicaParabolica(1400));
    racimo.setVelocidad(vx, vy);
    for (int i = 0; i < 600; i++) racimo.actualizar(dt);
    std::cout << "Racimo apuntado a (400, 580) cae en (" << racimo.getX() << ", " << racimo.getY() << ")" << std::endl;

    try {
        FisicaParabolica mala(0);
    } catch (const std::invalid_argument& e) {
        std::cout << "Gravedad 0 -> invalid_argument: " << e.what() << std::endl;
    }
    try {
        FisicaParabolica::velocidadApuntado(100, 0, 0, 1400, vx, vy);
    } catch (const std::invalid_argument& e) {
        std::cout << "Tiempo de vuelo 0 -> invalid_argument: " << e.what() << std::endl;
    }
    return 0;
}
