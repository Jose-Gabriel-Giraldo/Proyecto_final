#ifndef FISICAONDA_H
#define FISICAONDA_H

#include "modelofisico.h"

class FisicaOnda : public ModeloFisico {
public:
    FisicaOnda(double xCentro, double ySuelo, double v0, double k, double a0, double gamma, double r0 = 30);
    void aplicar(Entidad& e, double dt) override;

    double getRadio() const;
    double getAmplitud() const;
    double getVelocidadFrente() const;
    double radioMaximo() const;

private:
    double xCentro, ySuelo, v0, k, a0, gamma, r0, t;
};

#endif
