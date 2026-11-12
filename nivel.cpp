#include "nivel.h"
#include "proyectil.h"

Nivel::Nivel() : estado(NIVEL_EN_CURSO), puntaje(0), tiempo(0) {}

Nivel::~Nivel() {
    for (Proyectil* p : proyectiles) delete p;
}

int Nivel::getEstado() const { return estado; }
int Nivel::getPuntaje() const { return puntaje; }
double Nivel::getTiempo() const { return tiempo; }
const std::vector<Proyectil*>& Nivel::getProyectiles() const { return proyectiles; }

bool Nivel::extraerEvento(int& e) {
    if (eventos.empty()) return false;
    e = eventos.front();
    eventos.pop();
    return true;
}

void Nivel::agregarProyectil(Proyectil* p) {
    if (p) proyectiles.push_back(p);
}

void Nivel::actualizarProyectiles(double dt) {
    for (Proyectil* p : proyectiles) p->actualizar(dt);
}

void Nivel::limpiarProyectiles() {
    int i = 0;
    while (i < static_cast<int>(proyectiles.size())) {
        if (proyectiles[i]->estaActiva()) {
            i++;
            continue;
        }
        alEliminarProyectil(*proyectiles[i]);
        delete proyectiles[i];
        proyectiles[i] = proyectiles.back();
        proyectiles.pop_back();
    }
}

void Nivel::alEliminarProyectil(const Proyectil&) {}
