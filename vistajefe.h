#ifndef VISTAJEFE_H
#define VISTAJEFE_H

#include <QElapsedTimer>
#include <QPointF>
#include <QTimer>
#include <QWidget>
#include <map>
#include <vector>
#include "animacion.h"
#include "configuraciondificultad.h"

class NivelJefe;
class Proyectil;
#ifdef CON_MUSICA
class QMediaPlayer;
class QAudioOutput;
#endif

class VistaJefe : public QWidget {
    Q_OBJECT
public:
    VistaJefe(const ConfiguracionDificultad& config, QWidget* parent = nullptr);
    ~VistaJefe() override;

    void iniciar();
    void detener();
    void reiniciar();

signals:
    void nivelTerminado(bool gano, int puntaje);
    void salirAlMenu();

protected:
    void paintEvent(QPaintEvent*) override;
    void keyPressEvent(QKeyEvent* e) override;
    void keyReleaseEvent(QKeyEvent* e) override;
    void mousePressEvent(QMouseEvent* e) override;
    void mouseReleaseEvent(QMouseEvent* e) override;
    void mouseMoveEvent(QMouseEvent* e) override;

private:
    void paso();
    void sincronizarMusica();
    void actualizarEfectos(double dt);
    void agregarEfecto(QPointF pos, int tipo);
    void dibujarRitmo(QPainter& p);
    void dibujarPalmera(QPainter& p);
    void dibujarRick(QPainter& p);
    void dibujarMutantes(QPainter& p);
    void dibujarProyectiles(QPainter& p);
    void dibujarEfectos(QPainter& p);
    void dibujarHud(QPainter& p);
    int accionDeTecla(int tecla) const;
    std::vector<QString> opciones() const;
    void pausar(bool activa);
    void elegirOpcion();
    QPointF aEscena(QPointF punto) const;

    ConfiguracionDificultad config;
    NivelJefe* nivel;
    QTimer timer;
    QElapsedTimer reloj;
    double tiempo, temblor, golpePalmera, ataqueVisual, tiempoFin;
    int vidaPalmera, tipoAtaque, seleccion;
    bool disparando, terminado, pausado;
    QPointF mira;

    QPixmap fondo, racimo, racimoBrillo;
    Animacion rIdle, rCaminar;
    Animacion p1Idle, p1Ataque, p2Idle, p2Ataque;
    Animacion mCaminar, mAtacar, banano, hojas, impacto, onda;
    std::map<const Proyectil*, QPointF> racimos;
    std::map<const Proyectil*, bool> racimosExplotados;
    std::vector<QPointF> posEfectos;
    std::vector<int> tipoEfectos;
    std::vector<double> tiempoEfectos;

#ifdef CON_MUSICA
    QMediaPlayer* musica;
    QAudioOutput* salida;
#endif
};

#endif
