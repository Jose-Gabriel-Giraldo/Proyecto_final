#include "proyectil.h"
#include "modelofisico.h"
#include <cmath>

Proyectil::Proyectil(double x, double y, double vx, double vy, double ancho, double alto, int danio, bool delJugador,
                     int tipo, ModeloFisico* fisica, int idAtaque, double tiempoVida)
    : Entidad(x, y, ancho, alto, fisica), danio(danio), delJugador(delJugador), tipo(tipo), idAtaque(idAtaque),
    tiempoVida(tiempoVida), explotado(false) {
    this->vx = vx;
    this->vy = vy;
}

void Proyectil::actualizar(double dt) {
    if (!activa) return;
    if (fisica) fisica->aplicar(*this, dt);
    else setPosicion(x + vx * dt, y + vy * dt);
    tiempoVida -= dt;
    if (tiempoVida <= 0) activa = false;
    if (tipo == PROYECTIL_BANANO && std::hypot(vx, vy) < 40) activa = false;
}

void Proyectil::explotar(double radio) {
    double cx = getCentroX(), piso = y + alto;
    ancho = alto = 2 * radio;
    x = cx - radio;
    y = piso - alto;
    vx = vy = 0;
    setFisica(nullptr);
    tiempoVida = 0.25;
    explotado = true;
}

int Proyectil::getDanio() const { return danio; }
bool Proyectil::esDelJugador() const { return delJugador; }
int Proyectil::getTipo() const { return tipo; }
int Proyectil::getIdAtaque() const { return idAtaque; }
bool Proyectil::haExplotado() const { return explotado; }
