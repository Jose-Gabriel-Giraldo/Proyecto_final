#ifndef MODULOACCION_H
#define MODULOACCION_H

#include <random>
#include <vector>

class ModuloRazonamiento;
class ModuloAprendizaje;
class ConfiguracionDificultad;
class Proyectil;
class BananaMutante;

class ModuloAccion {
public:
    ModuloAccion(const ModuloRazonamiento& razonamiento, ModuloAprendizaje& aprendizaje, unsigned semilla);
    int ejecutar(double xBoca, double yBoca, double ySuelo, int mutantesVivos, const ConfiguracionDificultad& config,
                 std::vector<Proyectil*>& proyectiles, std::vector<BananaMutante*>& invocados);

private:
    const ModuloRazonamiento* razonamiento;
    ModuloAprendizaje* aprendizaje;
    int siguienteId;
    std::mt19937 generador;
};

#endif
