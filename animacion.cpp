#include "animacion.h"
#include <stdexcept>

Animacion::Animacion() : cuadros(1), fps(1), repetir(true), tiempo(0) {}

Animacion::Animacion(const QString& ruta, int cuadros, double fps, bool repetir)
    : hoja(ruta), cuadros(cuadros), fps(fps), repetir(repetir), tiempo(0) {
    if (hoja.isNull()) throw std::runtime_error("No se encontro la imagen " + ruta.toStdString());
    if (cuadros <= 0 || fps <= 0) throw std::invalid_argument("Animacion con cuadros o fps invalidos");
}

void Animacion::actualizar(double dt) {
    tiempo += dt;
    double duracion = cuadros / fps;
    if (repetir && tiempo >= duracion) tiempo -= duracion;
}

void Animacion::reiniciar() { tiempo = 0; }

void Animacion::setCuadro(int c) { tiempo = (c + 0.5) / fps; }

bool Animacion::terminada() const { return !repetir && tiempo * fps >= cuadros; }

int Animacion::getCuadroActual() const {
    int c = static_cast<int>(tiempo * fps);
    if (repetir) return c % cuadros;
    return c < cuadros ? c : cuadros - 1;
}

void Animacion::dibujar(QPainter& p, double centroX, double base, double alto, bool espejo) const {
    if (hoja.isNull()) return;
    double ancho = alto * getAnchoCuadro() / getAltoCuadro();
    QRectF origen(getCuadroActual() * getAnchoCuadro(), 0, getAnchoCuadro(), getAltoCuadro());
    QRectF destino(-ancho / 2, -alto, ancho, alto);
    p.save();
    p.translate(centroX, base);
    if (espejo) p.scale(-1, 1);
    p.drawPixmap(destino, hoja, origen);
    p.restore();
}

int Animacion::getCuadros() const { return cuadros; }
int Animacion::getAnchoCuadro() const { return hoja.width() / cuadros; }
int Animacion::getAltoCuadro() const { return hoja.height(); }
