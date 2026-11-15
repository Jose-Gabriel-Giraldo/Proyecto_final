#ifndef VISTAPLANTACION_H
#define VISTAPLANTACION_H

#include "hojasprites.h"
#include <QKeyEvent>
#include <QPaintEvent>
#include <QPixmap>
#include <QTimer>
#include <QWidget>

class NivelPlantacion;

// Capa grafica del nivel 1. No contiene reglas del juego: solo
//  1) traduce el teclado a acciones del nivel (procesarEntrada),
//  2) le pide al nivel que avance cada 16 ms (actualizar),
//  3) dibuja lo que el nivel dice que hay en el mundo.
// Toda la logica sigue viviendo en NivelPlantacion.
class VistaPlantacion : public QWidget {
    Q_OBJECT

public:
    explicit VistaPlantacion(QWidget* parent = nullptr);
    ~VistaPlantacion() override;

signals:
    void nivelTerminado(bool gano, int puntaje);

protected:
    void paintEvent(QPaintEvent* evento) override;
    void keyPressEvent(QKeyEvent* evento) override;
    void keyReleaseEvent(QKeyEvent* evento) override;

private slots:
    void actualizarJuego();

private:
    int teclaAAccion(int tecla) const;
    void reiniciar();
    void moverCamara();
    void leerEventos();

    void dibujarFondo(QPainter& p);
    void dibujarEscenario(QPainter& p);
    void dibujarEnemigos(QPainter& p);
    void dibujarMorty(QPainter& p);
    void dibujarProyectiles(QPainter& p);
    void dibujarHud(QPainter& p);
    void dibujarMensaje(QPainter& p, const QString& texto);

    // Imagenes (se cargan una sola vez en el constructor)
    QPixmap cielo, montanas, fondoNivel, plataforma;
    HojaSprites mortyQuieto, mortyCaminar, mortyCorrer, mortyAtacar, mortyDanado;
    HojaSprites bananaCaminar, bananaAtacar;
    HojaSprites soldadoQuieto, soldadoDisparar;
    HojaSprites jaulaQuieta, jaulaLiberar;

    NivelPlantacion* nivel;   // memoria dinamica: se crea con new y se libera en el destructor
    QTimer* temporizador;     // hijo del widget: Qt lo libera solo
    double camaraX;
    int cuadroAnimacion;      // contador que avanza solo para animar los sprites
    bool pausado, disparando, avisoEmitido;
    int temblor;              // cuadros que le quedan a la sacudida de camara al recibir dano
};

#endif
