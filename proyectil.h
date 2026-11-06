#ifndef PROYECTIL_H
#define PROYECTIL_H

#include "entidad.h"
#include "tipo.h"

class Proyectil : public Entidad {
public:
    Proyectil(double x, double y, double vx, double vy, double ancho, double alto, int danio, bool delJugador,
              int tipo, ModeloFisico* fisica = nullptr, int idAtaque = -1, double tiempoVida = 4.0);

    void actualizar(double dt) override;
    void explotar(double radio);

    int getDanio() const;
    bool esDelJugador() const;
    int getTipo() const;
    int getIdAtaque() const;
    bool haExplotado() const;

private:
    int danio;
    bool delJugador;
    int tipo;
    int idAtaque;
    double tiempoVida;
    bool explotado;
};

#endif
