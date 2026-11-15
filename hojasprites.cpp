#include "hojasprites.h"
#include <stdexcept>

QPixmap cargarImagen(const QString& ruta) {
    QPixmap img(ruta);
    if (img.isNull()) throw std::runtime_error("No se pudo cargar la imagen " + ruta.toStdString());
    return img;
}

HojaSprites::HojaSprites(const QString& ruta, int numCuadros) : imagen(cargarImagen(ruta)), numCuadros(numCuadros) {
    if (numCuadros <= 0) throw std::invalid_argument("Una hoja de sprites necesita al menos un cuadro");
    anchoCuadro = imagen.width() / numCuadros;
}

void HojaSprites::dibujar(QPainter& p, int cuadro, double centroX, double pies, double alto, bool espejo) const {
    int c = cuadro % numCuadros;
    QRectF origen(c * anchoCuadro, 0, anchoCuadro, imagen.height());
    double ancho = alto * anchoCuadro / imagen.height();

    p.save();
    p.translate(centroX, pies);      // el origen queda en los pies del personaje
    if (espejo) p.scale(-1, 1);      // voltear: lo que estaba a la derecha pasa a la izquierda
    p.drawPixmap(QRectF(-ancho / 2, -alto, ancho, alto), imagen, origen);
    p.restore();
}

int HojaSprites::getNumCuadros() const { return numCuadros; }
