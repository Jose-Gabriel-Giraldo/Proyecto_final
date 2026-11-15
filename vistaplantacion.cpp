#include "vistaplantacion.h"
#include "bananamutante.h"
#include "fisicapendular.h"
#include "morty.h"
#include "nivelplantacion.h"
#include "proyectil.h"
#include <QKeyEvent>
#include <QLinearGradient>
#include <QPainter>
#include <algorithm>
#include <cmath>
#include <cstdlib>
#include <stdexcept>

const double ESC_MORTY = 0.56;
const double ESC_BANANA = 0.62;
const double ESC_SOLDADO = 0.6;
const double ESC_JAULA = 0.62;
const double DISTANCIA_HUIDA = 180;

// Tramos del mundo armados con nivel1.png: {x destino, x origen, ancho}
const double PIEZAS[][3] = {{0, 0, 5580},        {5580, 1400, 292},  {5872, 2522, 1478},  {7350, 1400, 950},
                            {8300, 1400, 950},   {9250, 1400, 950},  {10200, 1400, 22},   {10222, 2522, 1478},
                            {11700, 1400, 522},  {12222, 2522, 1478}, {13700, 1400, 950}, {14650, 1400, 250},
                            {14900, 5580, 820}};

VistaPlantacion::VistaPlantacion(QWidget* parent)
    : QWidget(parent), nivel(nullptr), camX(0), temblor(0), tiempo(0), disparando(false), cielo(":/cielo.png"),
    montanas(":/montanas.png"), platanal(":/platanal.png"), fondo(":/nivel1.png"), plataforma(":/plataforma.png"),
    mIdle(":/morty_idle.png", 1, 1), mCaminar(":/morty_caminar.png", 3, 8), mCorrer(":/morty_correr.png", 3, 12),
    mAtacar(":/morty_atacar.png", 1, 1), mDanado(":/morty_danado.png", 1, 1), mLiana(":/morty_liana.png", 2, 3),
    bCaminar(":/banana_caminar.png", 3, 6), bAtacar(":/banana_atacar.png", 3, 10),
    sIdle(":/soldado_idle.png", 4, 6), sDisparar(":/soldado_disparar.png", 5, 12) {
    if (fondo.width() != 6400) throw std::runtime_error("nivel1.png debe ser la imagen original de 6400x720");
    setMinimumSize(640, 360);
    setFocusPolicy(Qt::StrongFocus);
    connect(&timer, &QTimer::timeout, this, &VistaPlantacion::paso);
    reiniciar();
}

VistaPlantacion::~VistaPlantacion() { delete nivel; }

void VistaPlantacion::iniciar() {
    reloj.start();
    timer.start(16);
    setFocus();
}

void VistaPlantacion::detener() { timer.stop(); }

void VistaPlantacion::reiniciar() {
    delete nivel;
    nivel = new NivelPlantacion();
    camX = 0;
    temblor = 0;
    tiempo = 0;
    disparando = false;

    const std::vector<double>& xs = nivel->getXJaulas();
    animJaulas.assign(xs.size(), Animacion(":/jaula_idle.png", 4, 4));
    faseJaulas.assign(xs.size(), 0);
    xTrabajadores.clear();
    for (double x : xs) xTrabajadores.push_back(x + 30);

    muertes.clear();
    vidaPrevia.clear();
    for (const BananaMutante* b : nivel->getBananas()) {
        if (b->esArmado()) muertes.push_back(Animacion(":/soldado_muerte.png", 4, 8, false));
        else muertes.push_back(Animacion(":/banana_danado.png", 1, 2, false));
        vidaPrevia.push_back(b->getVida());
    }
    golpeBananas.assign(muertes.size(), 0);
}

void VistaPlantacion::paso() {
    double dt = reloj.restart() / 1000.0;
    if (dt > 0.033) dt = 0.033;
    tiempo += dt;

    nivel->actualizar(dt);
    int e;
    while (nivel->extraerEvento(e))
        if (e == EVENTO_DANIO_JUGADOR) temblor = 0.25;
    if (temblor > 0) temblor -= dt;

    actualizarAnimaciones(dt);

    double objetivo = nivel->getMorty().getCentroX() - 1280 * 0.4;
    camX += (objetivo - camX) * std::min(1.0, dt * 8);
    if (camX < 0) camX = 0;
    if (camX > NivelPlantacion::ANCHO_MUNDO - 1280) camX = NivelPlantacion::ANCHO_MUNDO - 1280;
    update();
}

void VistaPlantacion::actualizarAnimaciones(double dt) {
    for (Animacion* a : {&mIdle, &mCaminar, &mCorrer, &mAtacar, &mDanado, &mLiana, &bCaminar, &bAtacar, &sIdle, &sDisparar})
        a->actualizar(dt);

    for (int i = 0; i < static_cast<int>(faseJaulas.size()); i++) {
        if (faseJaulas[i] == 0 && nivel->jaulaLiberada(i)) {
            faseJaulas[i] = 1;
            animJaulas[i] = Animacion(":/jaula_liberar.png", 4, 5, false);
        } else if (faseJaulas[i] == 1 && animJaulas[i].terminada()) {
            faseJaulas[i] = 2;
            animJaulas[i] = Animacion(":/trabajador_agradecer.png", 4, 6, false);
        } else if (faseJaulas[i] == 2 && animJaulas[i].terminada()) {
            faseJaulas[i] = 3;
            animJaulas[i] = Animacion(":/trabajador_huir.png", 4, 8);
        } else if (faseJaulas[i] == 3) {
            xTrabajadores[i] += 220 * dt;
            if (xTrabajadores[i] > nivel->getXJaulas()[i] + 30 + DISTANCIA_HUIDA) faseJaulas[i] = 4;
        }
        animJaulas[i].actualizar(dt);
    }

    const std::vector<BananaMutante*>& bananas = nivel->getBananas();
    for (int i = 0; i < static_cast<int>(bananas.size()); i++) {
        if (bananas[i]->getVida() < vidaPrevia[i]) golpeBananas[i] = 0.15;
        vidaPrevia[i] = bananas[i]->getVida();
        if (golpeBananas[i] > 0) golpeBananas[i] -= dt;
        if (!bananas[i]->estaActiva()) muertes[i].actualizar(dt);
    }
}

void VistaPlantacion::paintEvent(QPaintEvent*) {
    QPainter p(this);
    p.scale(width() / 1280.0, height() / 720.0);
    dibujarFondo(p);

    p.save();
    if (temblor > 0) p.translate(std::rand() % 9 - 4, std::rand() % 9 - 4);
    dibujarEscenario(p);
    dibujarJaulas(p);
    dibujarBananas(p);
    dibujarProyectiles(p);
    dibujarMorty(p);
    p.restore();

    dibujarHud(p);
}

void VistaPlantacion::dibujarFondo(QPainter& p) {
    p.drawPixmap(0, 0, cielo);
    for (int capa = 0; capa < 2; capa++) {
        const QPixmap& img = capa == 0 ? montanas : platanal;
        double factor = capa == 0 ? 0.2 : 0.5;
        int desfase = static_cast<int>(camX * factor) % img.width();
        for (int x = -desfase; x < 1280; x += img.width()) p.drawPixmap(x, 0, img);
    }
}

void VistaPlantacion::dibujarEscenario(QPainter& p) {
    for (const auto& pz : PIEZAS)
        if (pz[0] + pz[2] > camX && pz[0] < camX + 1280)
            p.drawPixmap(QRectF(pz[0] - camX, 0, pz[2], 720), fondo, QRectF(pz[1], 0, pz[2], 720));

    QLinearGradient tierra(0, 640, 0, 720);
    tierra.setColorAt(0, QColor(70, 46, 32));
    tierra.setColorAt(1, QColor(24, 16, 14));
    const std::vector<std::array<double, 2>>& huecos = nivel->getHuecos();
    for (int i = 1; i < static_cast<int>(huecos.size()); i++) {
        double x0 = huecos[i][0] + 24 - camX, ancho = huecos[i][1] - huecos[i][0] - 48;
        if (x0 + ancho < 0 || x0 > 1280) continue;
        p.fillRect(QRectF(x0, 641, ancho, 79), tierra);
        p.setPen(QPen(QColor(30, 24, 18), 2));
        p.setBrush(QColor(150, 120, 80));
        for (int k = 0; k * 20 < ancho - 12; k++) {
            double sx = x0 + k * 20 + 4, alto = 24 + (k * 37) % 20;
            QPointF punta[3] = {QPointF(sx, 720), QPointF(sx + 6, 720 - alto), QPointF(sx + 12, 720)};
            p.drawPolygon(punta, 3);
        }
    }

    for (const std::array<double, 4>& pl : nivel->getPlataformas())
        p.drawTiledPixmap(QRectF(pl[0] - camX, pl[1], pl[2], pl[3]), plataforma);

    const Morty& m = nivel->getMorty();
    p.setFont(QFont("Courier New", 12, QFont::Bold));
    for (const FisicaPendular* l : nivel->getLianas()) {
        QPointF a(l->getXAncla() - camX, l->getYAncla());
        QPointF b(l->getPuntoX() - camX, l->getPuntoY());
        p.setPen(QPen(QColor(30, 40, 20), 7));
        p.drawLine(a, b);
        p.setPen(QPen(QColor(90, 140, 55), 4));
        p.drawLine(a, b);
        p.setPen(QPen(QColor(30, 40, 20), 2));
        p.setBrush(QColor(100, 165, 65));
        p.drawEllipse(b, 9, 6);
        if (!m.getLiana() && std::hypot(l->getPuntoX() - m.getCentroX(), l->getPuntoY() - m.getCentroY()) < 80) {
            p.setPen(Qt::white);
            p.drawText(QRectF(b.x() - 80, b.y() - 34, 160, 20), Qt::AlignCenter, "E: agarrar");
        }
    }
}

void VistaPlantacion::dibujarJaulas(QPainter& p) {
    const std::vector<double>& xs = nivel->getXJaulas();
    const std::vector<double>& ys = nivel->getYJaulas();
    const std::vector<double>& prog = nivel->getProgresoJaulas();
    const Morty& m = nivel->getMorty();

    for (int i = 0; i < static_cast<int>(xs.size()); i++) {
        if (faseJaulas[i] == 4) continue;
        double cx = (faseJaulas[i] >= 2 ? xTrabajadores[i] : xs[i] + 30) - camX;
        if (cx < -200 || cx > 1480) continue;

        double ySuelo = ys[i];
        bool abriendo = faseJaulas[i] == 0 && prog[i] > 0;
        double sacudida = abriendo ? std::sin(tiempo * 60) * 2 : 0;
        if (faseJaulas[i] == 3) p.setOpacity(1 - (xTrabajadores[i] - xs[i] - 30) / DISTANCIA_HUIDA);
        animJaulas[i].dibujar(p, cx + sacudida, ySuelo, animJaulas[i].getAltoCuadro() * ESC_JAULA);
        p.setOpacity(1);

        if (abriendo) {
            QRectF barra(cx - 40, ySuelo - 175, 80, 8);
            p.setPen(Qt::NoPen);
            p.setBrush(QColor(0, 0, 0, 160));
            p.drawRect(barra);
            p.setBrush(QColor(110, 220, 90));
            p.drawRect(QRectF(barra.x(), barra.y(), 80 * prog[i] / NivelPlantacion::TIEMPO_JAULA, 8));
        } else if (faseJaulas[i] == 0 && std::fabs(xs[i] + 30 - m.getCentroX()) < 70 &&
                   std::fabs(m.getY() + m.getAlto() - ySuelo) < 1) {
            p.setPen(Qt::white);
            p.drawText(QRectF(cx - 100, ySuelo - 190, 200, 20), Qt::AlignCenter, "Mantén E para liberar");
        }
    }
}

void VistaPlantacion::dibujarBananas(QPainter& p) {
    const std::vector<BananaMutante*>& bananas = nivel->getBananas();
    for (int i = 0; i < static_cast<int>(bananas.size()); i++) {
        const BananaMutante* b = bananas[i];
        double cx = b->getCentroX() - camX;
        double base = b->getY() + b->getAlto();
        if (cx < -200 || cx > 1480) continue;
        bool espejo = b->getDireccion() < 0;
        double escala = b->esArmado() ? ESC_SOLDADO : ESC_BANANA;

        if (!b->estaActiva()) {
            if (b->esArmado() || !muertes[i].terminada())
                muertes[i].dibujar(p, cx, base, muertes[i].getAltoCuadro() * escala, espejo);
            continue;
        }

        if (golpeBananas[i] > 0) p.setOpacity(0.4);
        if (b->esArmado()) {
            Animacion& a = b->estaAlerta() ? sDisparar : sIdle;
            double desfase = 0;
            if (b->estaAlerta()) desfase = (sDisparar.getAnchoCuadro() - sIdle.getAnchoCuadro()) / 2.0 * escala * (espejo ? -1 : 1);
            a.dibujar(p, cx + desfase, base, a.getAltoCuadro() * escala, espejo);
        } else {
            Animacion& a = b->estaAlerta() ? bAtacar : bCaminar;
            a.dibujar(p, cx, base, a.getAltoCuadro() * escala, espejo);
        }
        p.setOpacity(1);
    }
}

void VistaPlantacion::dibujarMorty(QPainter& p) {
    const Morty& m = nivel->getMorty();
    if (m.esInvulnerable() && static_cast<int>(tiempo * 20) % 2) return;

    double cx = m.getCentroX() - camX;
    double base = m.getY() + m.getAlto();
    if (m.estaEnValentia()) {
        p.setPen(Qt::NoPen);
        p.setBrush(QColor(255, 220, 80, 90));
        p.drawEllipse(QPointF(cx, base - 50), 45, 62);
    }

    if (m.getLiana()) {
        const FisicaPendular* l = m.getLiana();
        double alto = mLiana.getAltoCuadro() * ESC_MORTY;
        p.save();
        p.translate(l->getPuntoX() - camX, l->getPuntoY());
        p.rotate(-l->getTheta() * 180 / PI);
        mLiana.dibujar(p, 0, alto - 12 * ESC_MORTY, alto, m.getDireccion() < 0);
        p.restore();
        return;
    }

    const Animacion* a = &mIdle;
    if (m.esInvulnerable()) a = &mDanado;
    else if (m.estaLiberando()) a = &mIdle;
    else if (disparando) a = &mAtacar;
    else if (!m.estaEnSuelo() || std::fabs(m.getVx()) > 300) a = &mCorrer;
    else if (std::fabs(m.getVx()) > 1) a = &mCaminar;
    a->dibujar(p, cx, base, a->getAltoCuadro() * ESC_MORTY, m.getDireccion() < 0);
}

void VistaPlantacion::dibujarProyectiles(QPainter& p) {
    for (const Proyectil* pr : nivel->getProyectiles()) {
        if (!pr->estaActiva()) continue;
        QRectF r(pr->getX() - camX, pr->getY(), pr->getAncho(), pr->getAlto());
        if (pr->getTipo() == PROYECTIL_DISPARO_MORTY) {
            p.setPen(QPen(QColor(20, 90, 30), 2));
            p.setBrush(QColor(130, 255, 120));
            p.drawEllipse(r);
        } else {
            p.setPen(QPen(QColor(90, 60, 0), 1));
            p.setBrush(QColor(255, 210, 60));
            p.drawRect(r);
        }
    }
}

void VistaPlantacion::dibujarHud(QPainter& p) {
    const Morty& m = nivel->getMorty();
    p.setPen(Qt::NoPen);
    p.setBrush(QColor(0, 0, 0, 140));
    p.drawRoundedRect(QRectF(12, 12, 430, 112), 8, 8);

    for (int i = 0; i < m.getVidaMax(); i++) {
        p.setBrush(i < m.getVida() ? QColor(230, 50, 60) : QColor(70, 40, 40));
        p.drawRoundedRect(QRectF(24 + i * 26, 22, 20, 20), 5, 5);
    }

    p.setFont(QFont("Courier New", 13, QFont::Bold));
    p.setPen(Qt::white);
    int seg = static_cast<int>(nivel->getTiempo());
    p.drawText(QPointF(170, 38), QString("Puntos: %1").arg(nivel->getPuntaje()));
    int total = static_cast<int>(nivel->getXJaulas().size());
    p.drawText(QPointF(24, 66), QString("Trabajadores: %1/%2 (mín. %3)")
                                    .arg(m.getLiberados()).arg(total).arg(NivelPlantacion::MINIMO_LIBERADOS));
    p.drawText(QPointF(330, 38), QString("%1:%2").arg(seg / 60).arg(seg % 60, 2, 10, QChar('0')));

    p.drawText(QPointF(24, 96), "Valor");
    double valor = m.estaEnValentia() ? 100 : std::min(100.0, m.getValor());
    p.setPen(Qt::NoPen);
    p.setBrush(QColor(60, 60, 60));
    p.drawRect(QRectF(90, 84, 200, 12));
    p.setBrush(m.estaEnValentia() ? QColor(255, 210, 60) : QColor(120, 180, 255));
    p.drawRect(QRectF(90, 84, 2 * valor, 12));
    p.setPen(m.estaCansado() ? QColor(170, 170, 170) : QColor(255, 210, 60));
    if (m.estaEnValentia()) p.drawText(QPointF(300, 96), "¡VALENTÍA!");
    else if (m.estaCansado()) p.drawText(QPointF(300, 96), "Cansado...");

    if (nivel->faltanTrabajadores()) {
        p.setPen(QColor(255, 230, 120));
        p.setFont(QFont("Courier New", 16, QFont::Bold));
        p.drawText(QRectF(0, 140, 1280, 30), Qt::AlignCenter,
                   QString("¡Debes liberar al menos %1 trabajadores!").arg(NivelPlantacion::MINIMO_LIBERADOS));
    }

    if (nivel->getEstado() == NIVEL_EN_CURSO) return;
    p.fillRect(QRectF(0, 0, 1280, 720), QColor(0, 0, 0, 160));
    bool gano = nivel->getEstado() == NIVEL_GANADO;
    p.setPen(gano ? QColor(255, 220, 90) : QColor(240, 80, 80));
    p.setFont(QFont("Courier New", 40, QFont::Bold));
    p.drawText(QRectF(0, 250, 1280, 60), Qt::AlignCenter, gano ? "¡LLEGASTE A LA ESTACIÓN!" : "MORTY HA CAÍDO");
    p.setPen(Qt::white);
    p.setFont(QFont("Courier New", 18, QFont::Bold));
    p.drawText(QRectF(0, 330, 1280, 30), Qt::AlignCenter, QString("Puntaje: %1").arg(nivel->getPuntaje()));
    p.drawText(QRectF(0, 380, 1280, 30), Qt::AlignCenter, "Presiona Enter para continuar");
}

int VistaPlantacion::accionDeTecla(int tecla) const {
    switch (tecla) {
    case Qt::Key_A: case Qt::Key_Left: return ACCION_IZQUIERDA;
    case Qt::Key_D: case Qt::Key_Right: return ACCION_DERECHA;
    case Qt::Key_W: case Qt::Key_Up: case Qt::Key_Space: return ACCION_SALTAR;
    case Qt::Key_Shift: return ACCION_CORRER;
    case Qt::Key_J: case Qt::Key_X: return ACCION_DISPARAR;
    case Qt::Key_E: return ACCION_INTERACTUAR;
    }
    return -1;
}

void VistaPlantacion::keyPressEvent(QKeyEvent* e) {
    if (e->isAutoRepeat()) return;
    if (nivel->getEstado() != NIVEL_EN_CURSO) {
        if (e->key() == Qt::Key_Return || e->key() == Qt::Key_Enter)
            emit nivelTerminado(nivel->getEstado() == NIVEL_GANADO, nivel->getPuntaje());
        return;
    }
    int a = accionDeTecla(e->key());
    if (a < 0) { QWidget::keyPressEvent(e); return; }
    if (a == ACCION_DISPARAR) disparando = true;
    nivel->procesarEntrada(a, true);
}

void VistaPlantacion::keyReleaseEvent(QKeyEvent* e) {
    if (e->isAutoRepeat()) return;
    int a = accionDeTecla(e->key());
    if (a < 0) { QWidget::keyReleaseEvent(e); return; }
    if (a == ACCION_DISPARAR) disparando = false;
    nivel->procesarEntrada(a, false);
}
