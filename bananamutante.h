#ifndef BANANAMUTANTE_H
#define BANANAMUTANTE_H

#include "personaje.h"

class BananaMutante : public Personaje {
public:
    BananaMutante(double x, double y, double xMin, double xMax, bool armado = false, double velocidad = 120.0,
                  double radio = 300.0, int vida = 3);

    void comportarse(const Personaje& objetivo, double dt) override;
    void actualizar(double dt) override;
    Proyectil* disparar() override;
    bool esArmado() const;
    bool estaAlerta() const;

private:
    double xMin, xMax, velocidad, radio, cadencia, recarga, objetivoX, objetivoY;
    bool armado, alerta;
};

#endif
