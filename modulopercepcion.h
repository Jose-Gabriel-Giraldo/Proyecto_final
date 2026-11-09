#ifndef MODULOPERCEPCION_H
#define MODULOPERCEPCION_H

#include "tipo.h"
#include <array>
#include <deque>

class Rick;

class ModuloPercepcion {
public:
    ModuloPercepcion(const Rick& rick);
    void observarFrame(double dt);
    void percibir();

    int getContexto() const;
    int getZona() const;
    int getEvasionPreferida() const;
    double getXRick() const;
    double getVelX() const;
    bool rickEnAire() const;
    double getPuntajeEvasion(int e) const;

private:
    const Rick* rick;
    std::deque<double> historialX;
    std::array<double, NUM_EVASIONES> puntajeEvasion;
    double dtFrame, tiempoPortalAnterior, xRick, velX;
    int zona;
    bool enAire;
    int evasionPreferida;
};

#endif
