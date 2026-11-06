#ifndef PERSONAJE_H
#define PERSONAJE_H

#include "entidad.h"

class Proyectil;

class Personaje : public Entidad {
public:
    Personaje(double x, double y, double ancho, double alto, int vida, ModeloFisico* fisica = nullptr);

    virtual void recibirDanio(int d);
    virtual Proyectil* disparar() = 0;
    virtual void comportarse(const Personaje& objetivo, double dt);
    void perderVida();
    void aterrizar(double ySuelo);

    bool estaVivo() const;
    bool estaEnSuelo() const;
    bool esInvulnerable() const;
    int getVida() const;
    int getVidaMax() const;
    int getDireccion() const;

protected:
    void actualizarInvulnerabilidad(double dt);

    int vida, vidaMax, direccion;
    bool enSuelo;
    double invulnerable, duracionInvulnerable;
};

#endif
