#include "modulopercepcion.h"
#include "rick.h"
#include <cmath>

ModuloPercepcion::ModuloPercepcion(const Rick& rick)
    : rick(&rick), puntajeEvasion{}, dtFrame(1.0 / 60), tiempoPortalAnterior(rick.getTiempoDesdePortal()),
    xRick(rick.getCentroX()), velX(0), zona(0), enAire(false), evasionPreferida(EVASION_NINGUNA) {}

void ModuloPercepcion::observarFrame(double dt) {
    if (dt <= 0) return;
    dtFrame = dt;
    double olvido = std::exp(-dt / 5.0);
    for (double& p : puntajeEvasion) p *= olvido;

    double tPortal = rick->getTiempoDesdePortal();
    if (tPortal < tiempoPortalAnterior) {
        puntajeEvasion[EVASION_PORTAL] += 0.6;
        historialX.clear();
    }
    tiempoPortalAnterior = tPortal;

    if (!historialX.empty() && std::fabs(rick->getCentroX() - historialX.back()) / dt > 150)
        puntajeEvasion[EVASION_LATERAL] += dt;
    if (!rick->estaEnSuelo()) puntajeEvasion[EVASION_SALTO] += dt;

    historialX.push_back(rick->getCentroX());
    if (historialX.size() > 30) historialX.pop_front();
}

void ModuloPercepcion::percibir() {
    xRick = rick->getCentroX();
    enAire = !rick->estaEnSuelo();
    velX = historialX.size() < 2 ? 0 : (historialX.back() - historialX.front()) / ((historialX.size() - 1) * dtFrame);

    zona = static_cast<int>((xRick - 40) / 240);
    if (zona < 0) zona = 0;
    if (zona > 2) zona = 2;

    evasionPreferida = EVASION_NINGUNA;
    double mejor = 0.3;
    for (int e = EVASION_SALTO; e < NUM_EVASIONES; e++)
        if (puntajeEvasion[e] > mejor) {
            mejor = puntajeEvasion[e];
            evasionPreferida = e;
        }
}

int ModuloPercepcion::getContexto() const { return zona * NUM_EVASIONES + evasionPreferida; }
int ModuloPercepcion::getZona() const { return zona; }
int ModuloPercepcion::getEvasionPreferida() const { return evasionPreferida; }
double ModuloPercepcion::getXRick() const { return xRick; }
double ModuloPercepcion::getVelX() const { return velX; }
bool ModuloPercepcion::rickEnAire() const { return enAire; }
double ModuloPercepcion::getPuntajeEvasion(int e) const { return puntajeEvasion[e]; }
