#ifndef ANIMACION_H
#define ANIMACION_H

#include <QPainter>
#include <QPixmap>
#include <QString>
#include <vector>

class Animacion {
public:
    Animacion();
    Animacion(const QString& ruta, int cuadros, double fps, bool repetir = true);

    void actualizar(double dt);
    void reiniciar();
    void setCuadro(int c);
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

void dibujarMenu(QPainter& p, const QString& titulo, const QColor& color, const QString& detalle,
                 const std::vector<QString>& opciones, int seleccion);

#endif
