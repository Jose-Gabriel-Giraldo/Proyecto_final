#include "entidad.h"
#include "modelofisico.h"

Entidad::Entidad(double x, double y, double ancho, double alto, ModeloFisico* fisica)
    : x(x), y(y), vx(0), vy(0), ancho(ancho), alto(alto), activa(true), fisica(fisica) {}

Entidad::~Entidad() { delete fisica; }

bool Entidad::colisionaCon(const Entidad& o) const {
    return activa && o.activa && x < o.x + o.ancho && o.x < x + ancho && y < o.y + o.alto && o.y < y + alto;
}

double Entidad::getX() const { return x; }
double Entidad::getY() const { return y; }
double Entidad::getVx() const { return vx; }
double Entidad::getVy() const { return vy; }
double Entidad::getAncho() const { return ancho; }
double Entidad::getAlto() const { return alto; }
double Entidad::getCentroX() const { return x + ancho / 2; }
double Entidad::getCentroY() const { return y + alto / 2; }
bool Entidad::estaActiva() const { return activa; }
ModeloFisico* Entidad::getFisica() const { return fisica; }

void Entidad::setPosicion(double nx, double ny) { x = nx; y = ny; }
void Entidad::setVelocidad(double nvx, double nvy) { vx = nvx; vy = nvy; }

void Entidad::setFisica(ModeloFisico* nueva) {
    if (nueva == fisica) return;
    delete fisica;
    fisica = nueva;
}

void Entidad::desactivar() { activa = false; }
