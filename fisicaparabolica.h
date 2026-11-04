#ifndef FISICAPARABOLICA_H
#define FISICAPARABOLICA_H

#include "modelofisico.h"

class FisicaParabolica : public ModeloFisico {
public:
    FisicaParabolica(double gravedad = 1400.0);
    void aplicar(Entidad& e, double dt) override;
    double getGravedad() const;
    static void velocidadApuntado(double dx, double dy, double T, double g, double& vx, double& vy);

private:
    double gravedad;
};

#endif
