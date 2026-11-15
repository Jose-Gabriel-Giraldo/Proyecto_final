#ifndef HOJASPRITES_H
#define HOJASPRITES_H

#include <QPainter>
#include <QPixmap>
#include <QString>

// Carga una imagen de los recursos. Si no existe lanza std::runtime_error,
// asi el juego no arranca "a ciegas" con sprites vacios.
QPixmap cargarImagen(const QString& ruta);

// Una hoja de sprites es una imagen con varios cuadros (frames) del mismo ancho
// puestos uno al lado del otro. Esta clase sabe recortar el cuadro que toca
// y dibujarlo escalado, con los pies del personaje en el punto indicado.
class HojaSprites {
public:
    HojaSprites(const QString& ruta, int numCuadros);

    // cuadro: numero de cuadro (se usa el modulo, asi se puede pasar un contador que crece siempre)
    // centroX, pies: punto (en coordenadas del mundo) donde van los pies del personaje
    // alto: alto en pixeles con el que se dibuja; el ancho se calcula para no deformar
    // espejo: true para voltear el sprite horizontalmente (mirar a la izquierda)
    void dibujar(QPainter& p, int cuadro, double centroX, double pies, double alto, bool espejo) const;

    int getNumCuadros() const;

private:
    QPixmap imagen;
    int numCuadros;
    int anchoCuadro;
};

#endif
