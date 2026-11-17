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

private:
    void paso();
    void sincronizarMusica();
    void actualizarEfectos(double dt);
    void dibujarRitmo(QPainter& p);
    void dibujarPalmera(QPainter& p);
    void dibujarRick(QPainter& p);
    void dibujarMutantes(QPainter& p);
    void dibujarProyectiles(QPainter& p);
    void dibujarHud(QPainter& p);
    int accionDeTecla(int tecla) const;
    std::vector<QString> opciones() const;
    void pausar(bool activa);
    void elegirOpcion();
    bool pausado;
    int seleccion;

    ConfiguracionDificultad config;
    NivelJefe* nivel;
    QTimer timer;
    QElapsedTimer reloj;
    double tiempo, temblor, golpePalmera, ataqueVisual;
    int vidaPalmera, tipoAtaque;
    bool disparando, terminado;

    QPixmap fondo, plataforma, banano, racimo, explosion;
    Animacion rIdle, rCaminar, rAtacar, rDanado;
    Animacion pIdle, pAtaques, pGolpe, pMuerte;
    Animacion mCaminar, mAtacar, hojas;
    std::map<const Proyectil*, QPointF> racimos;
    std::vector<QPointF> explosiones;
    std::vector<double> tiempoExplosiones;

#ifdef CON_MUSICA
    QMediaPlayer* musica;
    QAudioOutput* salida;
#endif
};

#endif
