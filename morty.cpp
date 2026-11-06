#include "morty.h"
#include "fisicaparabolica.h"
#include "fisicapendular.h"
#include "proyectil.h"
#include <cmath>

Morty::Morty(double x, double y)
    : Personaje(x, y, 50, 90, 5, new FisicaParabolica(1400.0)), valor(0), tiempoValentia(0), tiempoCansancio(0),
    recargaDisparo(0), enemigosCerca(0), liberados(0), dirMovimiento(0), corriendo(false), liberando(false),
    liana(nullptr), generador(std::random_device{}()) {}

void Morty::actualizar(double dt) {
    actualizarInvulnerabilidad(dt);
    recargaDisparo -= dt;
    if (tiempoValentia > 0) {
        tiempoValentia -= dt;
        if (tiempoValentia <= 0) { tiempoValentia = 0; tiempoCansancio = 4.0; }
    } else if (tiempoCansancio > 0) {
        tiempoCansancio -= dt;
        if (tiempoCansancio < 0) tiempoCansancio = 0;
    }

    if (liana) {
        if (dirMovimiento != 0 && dirMovimiento * liana->getOmega() >= 0) liana->impulsar(dirMovimiento * 1.2 * dt);
        x = liana->getPuntoX() - ancho / 2;
        y = liana->getPuntoY() - alto / 2;
        liana->velocidadTangencial(vx, vy);
        return;
    }

    double objetivo = liberando ? 0 : dirMovimiento * 260.0 * (corriendo ? 1.6 : 1.0) * (estaCansado() ? 0.7 : 1.0);
    if (enSuelo || liberando) vx = objetivo;
    else if (dirMovimiento != 0 && vx * dirMovimiento < std::fabs(objetivo)) vx = objetivo;
    if (dirMovimiento != 0) direccion = dirMovimiento;

    enSuelo = false;
    fisica->aplicar(*this, dt);
}

Proyectil* Morty::disparar() {
    if (liberando || liana || recargaDisparo > 0) return nullptr;
    recargaDisparo = 0.3;
    double s = calcularDispersion();
    double angulo = s > 0 ? std::uniform_real_distribution<double>(-s, s)(generador) * PI / 180 : 0;
    double v = 700;
    return new Proyectil(getCentroX(), getCentroY() - 10, direccion * v * std::cos(angulo), v * std::sin(angulo),
                         16, 8, 1, true, PROYECTIL_DISPARO_MORTY, nullptr, -1, 1.2);
}

void Morty::mover(int dir) { dirMovimiento = dir; }
void Morty::setCorrer(bool c) { corriendo = c; }

void Morty::saltar() {
    if (liana) { soltarLiana(); return; }
    if (!enSuelo || liberando) return;
    vy = estaEnValentia() ? -900.0 : -750.0;
    enSuelo = false;
}

void Morty::agarrarLiana(FisicaPendular* l) {
    liana = l;
    liberando = false;
    enSuelo = false;
}

void Morty::soltarLiana() { liana = nullptr; }

void Morty::setLiberando(bool l) {
    liberando = l && !liana;
    if (liberando) vx = 0;
}

void Morty::registrarLiberado() {
    liberados++;
    liberando = false;
    sumarValor(34);
}
void Morty::setEnemigosCerca(int n) { enemigosCerca = n; }

void Morty::sumarValor(double p) {
    if (estaEnValentia()) return;
    valor += p;
    if (valor >= 100) {
        valor = 0;
        tiempoValentia = 8.0;
        tiempoCansancio = 0;
    }
}

double Morty::calcularDispersion() const {
    if (estaEnValentia()) return 0;
    double s = 4.0 * enemigosCerca;
    return s > 25 ? 25 : s;
}

bool Morty::estaEnValentia() const { return tiempoValentia > 0; }
bool Morty::estaCansado() const { return tiempoCansancio > 0; }
bool Morty::estaLiberando() const { return liberando; }
double Morty::getValor() const { return valor; }
int Morty::getLiberados() const { return liberados; }
FisicaPendular* Morty::getLiana() const { return liana; }
