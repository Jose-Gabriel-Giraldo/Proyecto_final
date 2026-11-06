#include "bananamutante.h"
#include "proyectil.h"
#include <cmath>

BananaMutante::BananaMutante(double x, double y, double xMin, double xMax, bool armado, double velocidad, double radio,
                             int vida)
    : Personaje(x, y, 70, 110, vida), xMin(xMin), xMax(xMax), velocidad(velocidad), radio(radio), cadencia(1.8),
    recarga(1.8), armado(armado), alerta(false) {
    duracionInvulnerable = 0;
    direccion = -1;
    if (!armado) vx = -velocidad;
}

void BananaMutante::comportarse(const Personaje& objetivo, double) {
    double dx = objetivo.getCentroX() - getCentroX();
    double dy = objetivo.getCentroY() - getCentroY();

    if (armado) {
        alerta = std::fabs(dx) <= radio && std::fabs(dy) < 120;
        if (alerta) direccion = dx >= 0 ? 1 : -1;
        return;
    }

    alerta = std::fabs(dx) < radio && std::fabs(dy) < 150;
    if (alerta) vx = (dx > 0 ? 1 : -1) * velocidad * 1.5;
    else if (x <= xMin) vx = velocidad;
    else if (x + ancho >= xMax) vx = -velocidad;
    else if (std::fabs(vx) != velocidad) vx = vx >= 0 ? velocidad : -velocidad;
    direccion = vx >= 0 ? 1 : -1;
}

void BananaMutante::actualizar(double dt) {
    actualizarInvulnerabilidad(dt);
    if (armado) {
        if (recarga > 0) recarga -= dt;
        return;
    }
    x += vx * dt;
    if (x < xMin) x = xMin;
    if (x + ancho > xMax) x = xMax - ancho;
}

Proyectil* BananaMutante::disparar() {
    if (!armado || !alerta || recarga > 0) return nullptr;
    recarga = cadencia;
    double px = direccion > 0 ? x + ancho : x - 12;
    return new Proyectil(px, y + 50, direccion * 450.0, 0, 12, 6, 1, false, PROYECTIL_BALA_SOLDADO, nullptr, -1, 2.0);
}

bool BananaMutante::esArmado() const { return armado; }
bool BananaMutante::estaAlerta() const { return alerta; }
