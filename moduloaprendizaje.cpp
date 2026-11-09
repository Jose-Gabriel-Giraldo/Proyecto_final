#include "moduloaprendizaje.h"
#include <istream>
#include <ostream>
#include <stdexcept>

ModuloAprendizaje::ModuloAprendizaje() : usos{}, aciertos{} {
    for (auto& fila : pesos) fila.fill(0.5);
}

void ModuloAprendizaje::registrarPendiente(int id, int contexto, int ataque) {
    pendientes[id] = contexto * NUM_ATAQUES + ataque;
}

void ModuloAprendizaje::resolver(int id, bool impacto, double alfa) {
    auto it = pendientes.find(id);
    if (it == pendientes.end()) return;
    int c = it->second / NUM_ATAQUES, a = it->second % NUM_ATAQUES;
    pesos[c][a] += alfa * ((impacto ? 1.0 : 0.0) - pesos[c][a]);
    usos[a]++;
    if (impacto) aciertos[a]++;
    pendientes.erase(it);
}

double ModuloAprendizaje::getPeso(int contexto, int ataque) const {
    return pesos[contexto][ataque];
}

int ModuloAprendizaje::getUsos(int ataque) const { return usos[ataque]; }

double ModuloAprendizaje::getTasaAcierto(int ataque) const {
    return usos[ataque] == 0 ? 0.0 : static_cast<double>(aciertos[ataque]) / usos[ataque];
}

int ModuloAprendizaje::getPendientes() const { return static_cast<int>(pendientes.size()); }

void ModuloAprendizaje::guardar(std::ostream& out) const {
    for (const auto& fila : pesos) {
        for (double w : fila) out << w << ' ';
        out << '\n';
    }
}

void ModuloAprendizaje::cargar(std::istream& in) {
    std::array<std::array<double, NUM_ATAQUES>, CONTEXTOS> leidos;
    for (auto& fila : leidos)
        for (double& w : fila)
            if (!(in >> w) || w < 0 || w > 1)
                throw std::runtime_error("Memoria del agente danada o incompleta");
    pesos = leidos;
}
