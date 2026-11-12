#ifndef NIVELPLANTACION_H
#define NIVELPLANTACION_H

#include "nivel.h"
#include <array>
#include <vector>

class Morty;
class BananaMutante;
class FisicaPendular;

class NivelPlantacion : public Nivel {
public:
    static const double ANCHO_MUNDO;
    static const double ALTO_MUNDO;
    static const double Y_SUELO;
    static const double AGUA_INICIO;
    static const double AGUA_FIN;
    static const double X_ESTACION;
    static const int MINIMO_LIBERADOS = 3;
    static const double TIEMPO_JAULA;

    NivelPlantacion();
    ~NivelPlantacion() override;

    void actualizar(double dt) override;
    void procesarEntrada(int a, bool presionada) override;

    const Morty& getMorty() const;
    const std::vector<double>& getXJaulas() const;
    const std::vector<double>& getProgresoJaulas() const;
    bool jaulaLiberada(int i) const;
    const std::vector<BananaMutante*>& getBananas() const;
    const std::vector<FisicaPendular*>& getLianas() const;
    const std::vector<std::array<double, 4>>& getPlataformas() const;
    bool faltanTrabajadores() const;

private:
    void resolverTerreno(double piesAntes);
    void resolverLiberacion(double dt);
    void resolverCombate();

    Morty* morty;
    std::vector<double> xJaulas, progresoJaulas;
    std::vector<BananaMutante*> bananas;
    std::vector<FisicaPendular*> lianas;
    std::vector<std::array<double, 4>> plataformas;
    double checkpointX;
    bool izquierda, derecha, disparando, interactuar;
};

#endif
