#include "fisicaarrastre.h"
#include "entidad.h"
#include <cmath>
#include <stdexcept>

FisicaArrastre::FisicaArrastre(double coeficiente) : coeficiente(coeficiente) {
    if (coeficiente <= 0) throw std::invalid_argument("El coeficiente de arrastre debe ser positivo");
}

void FisicaArrastre::aplicar(Entidad& e, double dt) {
    double f = std::exp(-coeficiente * dt);
    double avance = (1 - f) / coeficiente;
    e.setPosicion(e.getX() + e.getVx() * avance, e.getY() + e.getVy() * avance);
    e.setVelocidad(e.getVx() * f, e.getVy() * f);
}

double FisicaArrastre::alcanceMaximo(double v0) const { return v0 / coeficiente; }
