#ifndef BANANAMUTANTE_H
#define BANANAMUTANTE_H

#include "personaje.h"

class BananaMutante : public Personaje {
public:
    BananaMutante(double x, double y, double xMin, double xMax, bool armado = false, double velocidad = 90.0,
                  double radio = 250.0, int vida = 2);

    void comportarse(const Personaje& objetivo, double dt) override;
    void actualizar(double dt) override;
    Proyectil* disparar() override;
    bool esArmado() const;
    bool estaAlerta() const;

private:
    double xMin, xMax, velocidad, radio, cadencia, recarga;
    bool armado, alerta;
};

#endif
