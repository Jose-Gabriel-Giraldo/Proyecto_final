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

void dibujarMenu(QPainter& p, const QString& titulo, const QColor& color, const QString& detalle,
                 const std::vector<QString>& opciones, int seleccion) {
    p.fillRect(QRectF(0, 0, 1280, 720), QColor(0, 0, 0, 170));
    double alto = 190 + 64.0 * opciones.size();
    QRectF caja(360, 360 - alto / 2, 560, alto);
    p.setPen(QPen(color, 3));
    p.setBrush(QColor(10, 14, 20, 235));
    p.drawRoundedRect(caja, 16, 16);

    p.setPen(color);
    p.setFont(QFont("Courier New", 28, QFont::Bold));
    p.drawText(QRectF(caja.x(), caja.y() + 20, caja.width(), 50), Qt::AlignCenter, titulo);
    p.setPen(Qt::white);
    p.setFont(QFont("Courier New", 16));
    p.drawText(QRectF(caja.x(), caja.y() + 75, caja.width(), 30), Qt::AlignCenter, detalle);

    for (int i = 0; i < static_cast<int>(opciones.size()); i++) {
        QRectF boton(caja.x() + 110, caja.y() + 120 + i * 64, 340, 48);
        bool elegido = i == seleccion;
        p.setPen(QPen(elegido ? color : QColor(110, 110, 110), 2));
        p.setBrush(elegido ? QColor(60, 130, 60, 230) : QColor(30, 34, 40, 230));
        p.drawRoundedRect(boton, 10, 10);
        p.setPen(Qt::white);
        p.setFont(QFont("Courier New", 18, QFont::Bold));
        p.drawText(boton, Qt::AlignCenter, (elegido ? "> " : "") + opciones[i]);
    }

    p.setPen(QColor(190, 190, 190));
    p.setFont(QFont("Courier New", 12));
    p.drawText(QRectF(caja.x(), caja.y() + alto - 34, caja.width(), 24), Qt::AlignCenter,
               "W/S o flechas para elegir - Enter para aceptar");
}
