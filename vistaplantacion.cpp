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
const double ANCLA_LIANA = 25;
const double DISTANCIA_HUIDA = 180;

// Tramos del mundo armados con nivel1.png: {x destino, x origen, ancho}
const double PIEZAS[][3] = {{0, 0, 5580},        {5580, 1400, 292},  {5872, 2522, 1478},  {7350, 1400, 950},
                            {8300, 1400, 950},   {9250, 1400, 950},  {10200, 1400, 22},   {10222, 2522, 1478},
                            {11700, 1400, 522},  {12222, 2522, 1478}, {13700, 1400, 950}, {14650, 1400, 250},
                            {14900, 5580, 820}};

VistaPlantacion::VistaPlantacion(QWidget* parent)
    : QWidget(parent), nivel(nullptr), camX(0), temblor(0), tiempo(0), tiempoLiana(0), tiempoAterrizaje(0),
    disparando(false), cayendoDeLiana(false), cielo(":/cielo.png"),
    montanas(":/montanas.png"), platanal(":/platanal.png"), fondo(":/nivel1.png"), plataforma(":/plataforma.png"),
    mIdle(":/morty_idle.png", 1, 1), mCaminar(":/morty_caminar.png", 3, 8), mCorrer(":/morty_correr.png", 3, 12),
    mAtacar(":/morty_atacar.png", 1, 1), mDanado(":/morty_danado.png", 1, 1), mLiana(":/morty_liana.png", 10, 1), mSoltar(":/morty_soltar.png", 3, 1),
    bCaminar(":/banana_caminar.png", 3, 6), bAtacar(":/banana_atacar.png", 3, 10),
    sIdle(":/soldado_idle.png", 4, 6), sDisparar(":/soldado_disparar.png", 5, 12) {
    if (fondo.width() != 6400) throw std::runtime_error("nivel1.png debe ser la imagen original de 6400x720");
    pausado = false;
    seleccion = 0;
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
    pausado = false;
    seleccion = 0;
    tiempoLiana = 0;
    tiempoAterrizaje = 0;
    cayendoDeLiana = false;

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
    if (pausado) {
        update();
        return;
    }
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
    const Morty& m = nivel->getMorty();
    if (m.getLiana()) {
        tiempoLiana += dt;
        cayendoDeLiana = true;
    } else {
        tiempoLiana = 0;
        if (cayendoDeLiana && m.estaEnSuelo()) {
            cayendoDeLiana = false;
            tiempoAterrizaje = 0.2;
        }
    }
    if (tiempoAterrizaje > 0) tiempoAterrizaje -= dt;

    for (Animacion* a : {&mIdle, &mCaminar, &mCorrer, &mAtacar, &mDanado, &bCaminar, &bAtacar, &sIdle, &sDisparar})
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

    bool espejo = m.getDireccion() < 0;
    if (m.getLiana()) {
        const FisicaPendular* l = m.getLiana();
        double th = std::fabs(l->getTheta());
        int c;
        if (tiempoLiana < 0.25) c = 0;
        else if (th < 0.1 && std::fabs(l->getOmega()) < 0.3) c = 1 + static_cast<int>(tiempo * 3) % 2;
        else {
            espejo = l->getOmega() < 0;
            c = th < 0.15 ? 9 : th < 0.3 ? 3 : th < 0.45 ? 4 : 5;
        }
        mLiana.setCuadro(c);
        double base = l->getPuntoY() + (mLiana.getAltoCuadro() - ANCLA_LIANA) * ESC_MORTY;
        mLiana.dibujar(p, l->getPuntoX() - camX, base, mLiana.getAltoCuadro() * ESC_MORTY, espejo);
        return;
    }
    if (cayendoDeLiana || tiempoAterrizaje > 0) {
        mSoltar.setCuadro(tiempoAterrizaje > 0 ? 2 : m.getVy() < 0 ? 0 : 1);
        mSoltar.dibujar(p, cx, base, mSoltar.getAltoCuadro() * ESC_MORTY, espejo);
        return;
    }

    const Animacion* a = &mIdle;
    if (m.esInvulnerable()) a = &mDanado;
    else if (m.estaLiberando()) a = &mIdle;
    else if (disparando) a = &mAtacar;
    else if (!m.estaEnSuelo() || std::fabs(m.getVx()) > 300) a = &mCorrer;
    else if (std::fabs(m.getVx()) > 1) a = &mCaminar;
    a->dibujar(p, cx, base, a->getAltoCuadro() * ESC_MORTY, espejo);
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

    if (pausado) {
        dibujarMenu(p, "PAUSA", QColor(255, 220, 120), QString("Puntaje: %1").arg(nivel->getPuntaje()), opciones(),
                    seleccion);
        return;
    }
    if (nivel->getEstado() == NIVEL_EN_CURSO) return;
    bool gano = nivel->getEstado() == NIVEL_GANADO;
    QString titulo = gano ? "¡LLEGASTE A LA ESTACIÓN!" : "MORTY HA CAÍDO";
    dibujarMenu(p, titulo, gano ? QColor(255, 220, 90) : QColor(240, 80, 80),
                QString("Puntaje: %1").arg(nivel->getPuntaje()), opciones(), seleccion);
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

std::vector<QString> VistaPlantacion::opciones() const {
    if (pausado) return {"Continuar", "Reiniciar nivel", "Salir al menú"};
    if (nivel->getEstado() == NIVEL_GANADO) return {"Continuar"};
    return {"Reintentar", "Salir al menú"};
}

void VistaPlantacion::pausar(bool activa) {
    pausado = activa;
    seleccion = 0;
    if (activa) {
        for (int a : {ACCION_IZQUIERDA, ACCION_DERECHA, ACCION_CORRER, ACCION_DISPARAR, ACCION_INTERACTUAR})
            nivel->procesarEntrada(a, false);
        disparando = false;

    } else {

    }
    setFocus();
    update();
}

void VistaPlantacion::elegirOpcion() {
    QString opcion = opciones()[seleccion];
    seleccion = 0;
    if (opcion == "Continuar") {
        if (pausado) pausar(false);
        else emit nivelTerminado(true, nivel->getPuntaje());
    } else if (opcion == "Salir al menú") {
        emit salirAlMenu();
    } else {
        reiniciar();
        iniciar();
    }
}

void VistaPlantacion::keyPressEvent(QKeyEvent* e) {
    if (e->isAutoRepeat()) return;
    int tecla = e->key();
    bool enMenu = pausado || nivel->getEstado() != NIVEL_EN_CURSO;
    if (!enMenu && (tecla == Qt::Key_P || tecla == Qt::Key_Escape)) {
        pausar(true);
        return;
    }
    if (enMenu) {
        int n = static_cast<int>(opciones().size());
        if (tecla == Qt::Key_W || tecla == Qt::Key_Up) seleccion = (seleccion + n - 1) % n;
        else if (tecla == Qt::Key_S || tecla == Qt::Key_Down) seleccion = (seleccion + 1) % n;
        else if (tecla == Qt::Key_Return || tecla == Qt::Key_Enter) elegirOpcion();
        else if (pausado && (tecla == Qt::Key_P || tecla == Qt::Key_Escape)) pausar(false);
        update();
        return;
    }
    int a = accionDeTecla(tecla);
    if (a < 0) {
        QWidget::keyPressEvent(e);
        return;
    }
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
