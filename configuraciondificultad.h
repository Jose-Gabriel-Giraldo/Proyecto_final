#ifndef CONFIGURACIONDIFICULTAD_H
#define CONFIGURACIONDIFICULTAD_H

#include <string>

class ConfiguracionDificultad {
public:
    ConfiguracionDificultad(const std::string& nombre, double tasaAprendizaje, double epsilon, int prediccion,
                            int avisoMs, double probContratiempo, double tiempoVueloRacimo);

    static ConfiguracionDificultad facil();
    static ConfiguracionDificultad normal();
    static ConfiguracionDificultad dificil();

    const std::string& getNombre() const;
    double getTasaAprendizaje() const;
    double getEpsilon() const;
    int getPrediccion() const;
    int getAvisoMs() const;
    double getProbContratiempo() const;
    double getTiempoVueloRacimo() const;

private:
    std::string nombre;
    double tasaAprendizaje, epsilon;
    int prediccion, avisoMs;
    double probContratiempo, tiempoVueloRacimo;
};

#endif
