#ifndef VISTAPLANTACION_H
#define VISTAPLANTACION_H

#include <QElapsedTimer>
#include <QTimer>
#include <QWidget>
#include <vector>
#include "animacion.h"

class NivelPlantacion;

class VistaPlantacion : public QWidget {
    Q_OBJECT
public:
    VistaPlantacion(QWidget* parent = nullptr);
    ~VistaPlantacion() override;

    void iniciar();
    void detener();
    void reiniciar();

signals:
    void nivelTerminado(bool gano, int puntaje);

protected:
    void paintEvent(QPaintEvent*) override;
    void keyPressEvent(QKeyEvent* e) override;
    void keyReleaseEvent(QKeyEvent* e) override;

private:
    void paso();
    void actualizarAnimaciones(double dt);
    void dibujarFondo(QPainter& p);
    void dibujarEscenario(QPainter& p);
    void dibujarJaulas(QPainter& p);
    void dibujarBananas(QPainter& p);
    void dibujarMorty(QPainter& p);
    void dibujarProyectiles(QPainter& p);
    void dibujarHud(QPainter& p);
    int accionDeTecla(int tecla) const;

    NivelPlantacion* nivel;
    QTimer timer;
    QElapsedTimer reloj;
    double camX, temblor, tiempo;
    double tiempoLiana, tiempoAterrizaje;
    bool disparando, cayendoDeLiana;

    QPixmap cielo, montanas, platanal, fondo, plataforma;
    Animacion mIdle, mCaminar, mCorrer, mAtacar, mDanado, mLiana, mSoltar;
    Animacion bCaminar, bAtacar, sIdle, sDisparar;
    std::vector<Animacion> animJaulas, muertes;
    std::vector<int> faseJaulas, vidaPrevia;
    std::vector<double> xTrabajadores, golpeBananas;
};

#endif
