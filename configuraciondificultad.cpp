#include "configuraciondificultad.h"
#include <stdexcept>

ConfiguracionDificultad::ConfiguracionDificultad(const std::string& nombre, double tasaAprendizaje, double epsilon,
                                                 int prediccion, int avisoMs, double probContratiempo,
                                                 double tiempoVueloRacimo)
    : nombre(nombre), tasaAprendizaje(tasaAprendizaje), epsilon(epsilon), prediccion(prediccion), avisoMs(avisoMs),
    probContratiempo(probContratiempo), tiempoVueloRacimo(tiempoVueloRacimo) {
    if (tasaAprendizaje <= 0 || tasaAprendizaje > 1 || epsilon < 0 || prediccion < 0 || prediccion > 2 ||
        avisoMs < 0 || probContratiempo < 0 || probContratiempo > 1 || tiempoVueloRacimo <= 0)
        throw std::invalid_argument("Configuracion de dificultad invalida: " + nombre);
}

ConfiguracionDificultad ConfiguracionDificultad::facil() { return {"Facil", 0.05, 0.40, 0, 450, 0.0, 1.2}; }
ConfiguracionDificultad ConfiguracionDificultad::normal() { return {"Normal", 0.15, 0.20, 1, 350, 0.15, 1.0}; }
ConfiguracionDificultad ConfiguracionDificultad::dificil() { return {"Dificil", 0.30, 0.08, 2, 250, 0.35, 0.8}; }

const std::string& ConfiguracionDificultad::getNombre() const { return nombre; }
double ConfiguracionDificultad::getTasaAprendizaje() const { return tasaAprendizaje; }
double ConfiguracionDificultad::getEpsilon() const { return epsilon; }
int ConfiguracionDificultad::getPrediccion() const { return prediccion; }
int ConfiguracionDificultad::getAvisoMs() const { return avisoMs; }
double ConfiguracionDificultad::getProbContratiempo() const { return probContratiempo; }
double ConfiguracionDificultad::getTiempoVueloRacimo() const { return tiempoVueloRacimo; }
