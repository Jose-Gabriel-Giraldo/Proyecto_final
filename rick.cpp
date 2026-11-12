#include "rick.h"
#include "fisicaparabolica.h"
#include "proyectil.h"
#include <cmath>

Rick::Rick(double x, double y, double xMin, double xMax)
    : Personaje(x, y, 60, 120, 8, new FisicaParabolica(1400.0)), combo(0), dirMovimiento(0), multiplicador(1.0),
    recargaPortal(0), aturdido(0), recargaDisparo(0), tiempoDesdePortal(100), xMin(xMin), xMax(xMax) {}

void Rick::actualizar(double dt) {
    actualizarInvulnerabilidad(dt);
    recargaPortal = recargaPortal > dt ? recargaPortal - dt : 0;
    aturdido = aturdido > dt ? aturdido - dt : 0;
    recargaDisparo -= dt;
    tiempoDesdePortal += dt;

    vx = dirMovimiento * 300.0;
    if (dirMovimiento != 0) direccion = dirMovimiento;
    enSuelo = false;
    fisica->aplicar(*this, dt);
    if (x < xMin) x = xMin;
    if (x > xMax) x = xMax;
}

Proyectil* Rick::disparar() {
    if (aturdido > 0 || recargaDisparo > 0) return nullptr;
    recargaDisparo = 0.25;
    int danio = static_cast<int>(std::lround(10 * multiplicador));
    return new Proyectil(x + ancho, y + 45, 900.0, 0, 30, 10, danio, true, PROYECTIL_RAYO_RICK,
                         nullptr, -1, 1.2);
}

void Rick::recibirDanio(int d) {
    int antes = vida;
    Personaje::recibirDanio(d);
    if (vida < antes) {
        combo = 0;
        multiplicador = 1.0;
        aturdido = 0.6;
    }
}

void Rick::mover(int dir) { dirMovimiento = dir; }

void Rick::saltar() {
    if (!enSuelo) return;
    vy = -750.0;
    enSuelo = false;
}

bool Rick::teletransportar() {
    if (recargaPortal > 0) return false;
    x += direccion * 220.0;
    if (x < xMin) x = xMin;
    if (x > xMax) x = xMax;
    if (invulnerable < 0.25) invulnerable = 0.25;
    recargaPortal = 3.0;
    tiempoDesdePortal = 0;
    return true;
}

void Rick::registrarImpacto() {
    combo++;
    multiplicador = 1.0 + 0.1 * combo;
    if (multiplicador > 2.0) multiplicador = 2.0;
}

int Rick::getCombo() const { return combo; }
double Rick::getMultiplicador() const { return multiplicador; }
double Rick::getRecargaPortal() const { return recargaPortal; }
double Rick::getTiempoDesdePortal() const { return tiempoDesdePortal; }
bool Rick::estaAturdido() const { return aturdido > 0; }
