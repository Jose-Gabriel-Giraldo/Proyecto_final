#include "fisicapendular.h"
#include "entidad.h"
#include <cmath>
#include <stdexcept>

FisicaPendular::FisicaPendular(double xAncla, double yAncla, double longitud, double thetaInicial,
                               double amortiguamiento, double gravedad, double velAncla)
    : xAncla(xAncla), yAncla(yAncla), longitud(longitud), theta(thetaInicial), omega(0),
    amortiguamiento(amortiguamiento), gravedad(gravedad), velAncla(velAncla) {
    if (longitud <= 0 || gravedad <= 0 || amortiguamiento < 0)
        throw std::invalid_argument("Parametros invalidos para el pendulo");
}

void FisicaPendular::avanzar(double dt) {
    double alfa = -(gravedad / longitud) * std::sin(theta) - amortiguamiento * omega;
    omega += alfa * dt;
    theta += omega * dt;
    yAncla += velAncla * dt;
}

void FisicaPendular::aplicar(Entidad& e, double dt) {
    avanzar(dt);
    double vx, vy;
    velocidadTangencial(vx, vy);
    e.setVelocidad(vx, vy);
    e.setPosicion(getPuntoX() - e.getAncho() / 2, getPuntoY() - e.getAlto() / 2);
}

void FisicaPendular::impulsar(double dOmega) { omega += dOmega; }

void FisicaPendular::velocidadTangencial(double& vx, double& vy) const {
    vx = longitud * omega * std::cos(theta);
    vy = -longitud * omega * std::sin(theta) + velAncla;
}

double FisicaPendular::getPuntoX() const { return xAncla + longitud * std::sin(theta); }
double FisicaPendular::getPuntoY() const { return yAncla + longitud * std::cos(theta); }
double FisicaPendular::getXAncla() const { return xAncla; }
double FisicaPendular::getYAncla() const { return yAncla; }
double FisicaPendular::getTheta() const { return theta; }
double FisicaPendular::getOmega() const { return omega; }
double FisicaPendular::getLongitud() const { return longitud; }
