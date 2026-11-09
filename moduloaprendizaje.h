#ifndef MODULOAPRENDIZAJE_H
#define MODULOAPRENDIZAJE_H

#include "tipo.h"
#include <array>
#include <iosfwd>
#include <map>

class ModuloAprendizaje {
public:
    static const int CONTEXTOS = 12;

    ModuloAprendizaje();
    void registrarPendiente(int id, int contexto, int ataque);
    void resolver(int id, bool impacto, double alfa);

    double getPeso(int contexto, int ataque) const;
    int getUsos(int ataque) const;
    double getTasaAcierto(int ataque) const;
    int getPendientes() const;

    void guardar(std::ostream& out) const;
    void cargar(std::istream& in);

private:
    std::array<std::array<double, NUM_ATAQUES>, CONTEXTOS> pesos;
    std::map<int, int> pendientes;
    std::array<int, NUM_ATAQUES> usos, aciertos;
};

#endif
