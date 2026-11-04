#ifndef FISICAPENDULAR_H
#define FISICAPENDULAR_H

#include "modelofisico.h"

class FisicaPendular : public ModeloFisico {
public:
    FisicaPendular(double xAncla, double yAncla, double longitud, double thetaInicial = 0.0,
                   double amortiguamiento = 0.6, double gravedad = 1400.0, double velAncla = 0.0);
    void aplicar(Entidad& e, double dt) override;
    void avanzar(double dt);
    void impulsar(double dOmega);
    void velocidadTangencial(double& vx, double& vy) const;
    double getPuntoX() const;
    double getPuntoY() const;
    double getXAncla() const;
    double getYAncla() const;
    double getTheta() const;
    double getOmega() const;
    double getLongitud() const;

private:
    double xAncla, yAncla, longitud, theta, omega, amortiguamiento, gravedad, velAncla;
};

#endif
