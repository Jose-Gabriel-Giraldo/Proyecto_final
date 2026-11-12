#ifndef PALMERAJEFE_H
#define PALMERAJEFE_H

#include "personaje.h"
#include "tipo.h"
#include <vector>

class Rick;
class BananaMutante;
class ConfiguracionDificultad;
class ModuloPercepcion;
class ModuloRazonamiento;
class ModuloAccion;
class ModuloAprendizaje;

class PalmeraJefe : public Personaje {
public:
    PalmeraJefe(double x, double y, const Rick& rick, const ConfiguracionDificultad& config, unsigned semilla);
    ~PalmeraJefe() override;

    void actualizar(double dt) override;
    Proyectil* disparar() override;
    void recibirDanio(int d) override;

    void preparar(long long tiempoMs, bool contratiempo, int mutantesVivos);
    int lanzar(int mutantesVivos, std::vector<Proyectil*>& proyectiles, std::vector<BananaMutante*>& invocados);
    void notificarResultado(int idAtaque, bool impacto);

    int getFase() const;
    bool tieneAtaquePendiente() const;
    long long getTiempoPendiente() const;
    int getAtaquePendiente() const;
    const ModuloPercepcion& getPercepcion() const;
    const ModuloAprendizaje& getAprendizaje() const;
    ModuloAprendizaje& getAprendizaje();

private:
    int fase;
    const ConfiguracionDificultad* config;
    ModuloAprendizaje* aprendizaje;
    ModuloPercepcion* percepcion;
    ModuloRazonamiento* razonamiento;
    ModuloAccion* accion;
    bool pendiente;
    long long tiempoPendiente;
};

#endif
