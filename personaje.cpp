#include "personaje.h"

Personaje::Personaje(double x, double y, double ancho, double alto, int vida, ModeloFisico* fisica)
    : Entidad(x, y, ancho, alto, fisica), vida(vida), vidaMax(vida), direccion(1), enSuelo(false),
    invulnerable(0), duracionInvulnerable(1.0) {}

void Personaje::recibirDanio(int d) {
    if (invulnerable > 0 || vida <= 0 || d <= 0) return;
    vida -= d;
    if (vida < 0) vida = 0;
    invulnerable = duracionInvulnerable;
}

void Personaje::comportarse(const Personaje&, double) {}

void Personaje::perderVida() {
    if (vida > 0) vida--;
    invulnerable = duracionInvulnerable;
}

void Personaje::aterrizar(double ySuelo) {
    y = ySuelo - alto;
    if (vy > 0) vy = 0;
    enSuelo = true;
}

void Personaje::actualizarInvulnerabilidad(double dt) {
    invulnerable -= dt;
    if (invulnerable < 0) invulnerable = 0;
}

bool Personaje::estaVivo() const { return vida > 0; }
bool Personaje::estaEnSuelo() const { return enSuelo; }
bool Personaje::esInvulnerable() const { return invulnerable > 0; }
int Personaje::getVida() const { return vida; }
int Personaje::getVidaMax() const { return vidaMax; }
int Personaje::getDireccion() const { return direccion; }
