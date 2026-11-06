#ifndef RICK_H
#define RICK_H

#include "personaje.h"

class Rick : public Personaje {
public:
    Rick(double x, double y, double xMin, double xMax);

    void actualizar(double dt) override;
    Proyectil* disparar() override;
    void recibirDanio(int d) override;

    void mover(int dir);
    void saltar();
    bool teletransportar();
    void registrarImpacto();

    int getCombo() const;
    double getMultiplicador() const;
    double getRecargaPortal() const;
    double getTiempoDesdePortal() const;
    bool estaAturdido() const;

private:
    int combo, dirMovimiento;
    double multiplicador, recargaPortal, aturdido, recargaDisparo, tiempoDesdePortal, xMin, xMax;
};

#endif
