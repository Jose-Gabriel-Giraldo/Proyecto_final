#ifndef NIVELJEFE_H
#define NIVELJEFE_H

#include "configuraciondificultad.h"
#include "nivel.h"
#include <array>
#include <iosfwd>
#include <map>
#include <queue>
#include <random>
#include <vector>

class Rick;
class PalmeraJefe;
class BananaMutante;

class NivelJefe : public Nivel {
public:
    static const double ANCHO_PANTALLA;
    static const double ALTO_PANTALLA;
    static const double Y_SUELO;

    NivelJefe(const ConfiguracionDificultad& config, unsigned semilla = std::random_device{}());
    ~NivelJefe() override;

    void cargarMapaRitmo(std::istream& in);
    void sincronizar(long long posMs);
    void actualizar(double dt) override;
    void procesarEntrada(int a, bool presionada) override;

    const Rick& getRick() const;
    const PalmeraJefe& getPalmera() const;
    PalmeraJefe& getPalmera();
    const std::vector<BananaMutante*>& getMutantes() const;
    const ConfiguracionDificultad& getConfiguracion() const;
    double getPosicionCancionMs() const;
    long long getDuracionMs() const;
    double getMsPorBeat() const;
    int getIntensidadActual() const;
    const std::vector<std::array<double, 4>>& getPlataformas() const;

private:
    void procesarRitmo();
    void lanzarAtaque(bool programarContratiempo);
    void resolverColisiones();
    void alEliminarProyectil(const Proyectil& p) override;

    ConfiguracionDificultad config;
    Rick* rick;
    PalmeraJefe* palmera;
    std::vector<BananaMutante*> mutantes;
    std::vector<std::array<double, 4>> plataformas;
    std::queue<long long> beats;
    std::queue<int> intensidades;
    std::map<int, int> vivosPorAtaque;
    double msPorBeat, posCancionMs, proximoContratiempo;
    long long duracionMs;
    int contadorBeats, idAtaqueMutantes, intensidadActual;
    bool izquierda, derecha, disparando;
    std::mt19937 generador;
};

#endif
