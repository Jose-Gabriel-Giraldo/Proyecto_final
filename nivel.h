#ifndef NIVEL_H
#define NIVEL_H

#include "tipo.h"
#include <queue>
#include <vector>

class Proyectil;

class Nivel {
public:
    Nivel();
    virtual ~Nivel();
    Nivel(const Nivel&) = delete;
    Nivel& operator=(const Nivel&) = delete;

    virtual void actualizar(double dt) = 0;
    virtual void procesarEntrada(int a, bool presionada) = 0;

    int getEstado() const;
    int getPuntaje() const;
    double getTiempo() const;
    const std::vector<Proyectil*>& getProyectiles() const;
    bool extraerEvento(int& e);

protected:
    void agregarProyectil(Proyectil* p);
    void actualizarProyectiles(double dt);
    void limpiarProyectiles();
    virtual void alEliminarProyectil(const Proyectil& p);

    std::vector<Proyectil*> proyectiles;
    std::queue<int> eventos;
    int estado;
    int puntaje;
    double tiempo;
};

#endif
