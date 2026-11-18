#include "fisicaonda.h"
#include "entidad.h"
#include <cmath>
#include <stdexcept>

FisicaOnda::FisicaOnda(double xCentro, double ySuelo, double v0, double k, double a0, double gamma, double r0)
    : xCentro(xCentro), ySuelo(ySuelo), v0(v0), k(k), a0(a0), gamma(gamma), r0(r0), t(0) {
    if (v0 <= 0 || k <= 0 || a0 <= 0 || gamma < 0 || r0 <= 0)
        throw std::invalid_argument("Parametros invalidos para la onda expansiva");
}

void FisicaOnda::aplicar(Entidad& e, double dt) {
    t += dt;
    double r = getRadio(), a = getAmplitud();
    e.setTamano(2 * r, a);
    e.setPosicion(xCentro - r, ySuelo - a);
    e.setVelocidad(getVelocidadFrente(), 0);
}

double FisicaOnda::getRadio() const { return r0 + v0 / k * (1 - std::exp(-k * t)); }
double FisicaOnda::getAmplitud() const { return a0 * std::sqrt(r0 / getRadio()) * std::exp(-gamma * t); }
double FisicaOnda::getVelocidadFrente() const { return v0 * std::exp(-k * t); }
double FisicaOnda::radioMaximo() const { return r0 + v0 / k; }
