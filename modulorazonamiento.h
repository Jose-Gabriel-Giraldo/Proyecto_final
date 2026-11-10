#ifndef MODULORAZONAMIENTO_H
#define MODULORAZONAMIENTO_H

#include "tipo.h"
#include <random>

class ModuloPercepcion;
class ModuloAprendizaje;
class ConfiguracionDificultad;

class ModuloRazonamiento {
public:
    ModuloRazonamiento(const ModuloPercepcion& percepcion, const ModuloAprendizaje& aprendizaje, unsigned semilla);
    void decidir(int fase, bool contratiempo, int mutantesVivos, const ConfiguracionDificultad& config);

    int getAtaque() const;
    double getObjetivoX() const;
    int getContexto() const;

private:
    const ModuloPercepcion* percepcion;
    const ModuloAprendizaje* aprendizaje;
    int ataque;
    double objetivoX;
    int contexto;
    std::mt19937 generador;
};

#endif
