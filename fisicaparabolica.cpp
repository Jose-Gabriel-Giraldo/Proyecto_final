#include "fisicaparabolica.h"
#include "entidad.h"
#include <stdexcept>

FisicaParabolica::FisicaParabolica(double gravedad) : gravedad(gravedad) {
    if (gravedad <= 0) throw std::invalid_argument("La gravedad debe ser positiva");
}

void FisicaParabolica::aplicar(Entidad& e, double dt) {
    double vy = e.getVy() + gravedad * dt;
    e.setVelocidad(e.getVx(), vy);
    e.setPosicion(e.getX() + e.getVx() * dt, e.getY() + vy * dt);
}

double FisicaParabolica::getGravedad() const { return gravedad; }

void FisicaParabolica::velocidadApuntado(double dx, double dy, double T, double g, double& vx, double& vy) {
    if (T <= 0) throw std::invalid_argument("El tiempo de vuelo debe ser positivo");
    vx = dx / T;
    vy = (dy - 0.5 * g * T * T) / T;
}
