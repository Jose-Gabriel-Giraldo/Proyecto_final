#ifndef ANIMACION_H
#define ANIMACION_H

#include <QPainter>
#include <QPixmap>
#include <QString>

class Animacion {
public:
    Animacion();
    Animacion(const QString& ruta, int cuadros, double fps, bool repetir = true);

    void actualizar(double dt);
    void reiniciar();
    bool terminada() const;
    void dibujar(QPainter& p, double centroX, double base, double alto, bool espejo = false) const;

    int getCuadroActual() const;
    int getCuadros() const;
    int getAnchoCuadro() const;
    int getAltoCuadro() const;

private:
    QPixmap hoja;
    int cuadros;
    double fps;
    bool repetir;
    double tiempo;
};

#endif
