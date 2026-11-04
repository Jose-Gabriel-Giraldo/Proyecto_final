#ifndef FISICAARRASTRE_H
#define FISICAARRASTRE_H

#include "modelofisico.h"

class FisicaArrastre : public ModeloFisico {
public:
    FisicaArrastre(double coeficiente);
    void aplicar(Entidad& e, double dt) override;
    double alcanceMaximo(double v0) const;

private:
    double coeficiente;
};

#endif
