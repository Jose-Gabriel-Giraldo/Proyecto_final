#ifndef MODELOFISICO_H
#define MODELOFISICO_H

class Entidad;

class ModeloFisico {
public:
    virtual ~ModeloFisico() = default;
    virtual void aplicar(Entidad& e, double dt) = 0;
};

#endif
