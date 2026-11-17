#include "vistajefe.h"
#include "bananamutante.h"
#include "niveljefe.h"
#include "palmerajefe.h"
#include "proyectil.h"
#include "rick.h"
#include <QFile>
#include <QKeyEvent>
#include <QPainter>
#include <cmath>
#include <cstdlib>
#include <sstream>
#include <stdexcept>
#ifdef CON_MUSICA
#include <QAudioOutput>
#include <QMediaPlayer>
#include <QUrl>
#endif

const double ESC_RICK = 0.6;
const double ESC_MUTANTE = 0.62;
const double ALTO_PALMERA = 440;

VistaJefe::VistaJefe(const ConfiguracionDificultad& config, QWidget* parent)
    : QWidget(parent), config(config), nivel(nullptr), tiempo(0), temblor(0), golpePalmera(0), ataqueVisual(0),
    vidaPalmera(0), tipoAtaque(0), disparando(false), terminado(false), fondo(":/nivel2.png"),
    plataforma(":/plataforma.png"), banano(":/proyectiles_banano.png"), racimo(":/proyectiles_racimo.png"), explosion(":/proyectiles_explosion.png"),
    rIdle(":/rick_idle.png", 1, 1), rCaminar(":/rick_caminar.png", 3, 8), rAtacar(":/rick_atacar.png", 1, 1),
    rDanado(":/rick_danado.png", 1, 1), pIdle(":/palmera_idle.png", 1, 1), pAtaques(":/palmera_ataques.png", 4, 1),
    pGolpe(":/palmera_golpe.png", 4, 12, false), pMuerte(":/palmera_muerte.png", 3, 2, false),
    mCaminar(":/banana_caminar.png", 3, 6), mAtacar(":/banana_atacar.png", 3, 10), hojas(":/proyectiles_hojas.png", 3, 8) {
    pausado = false;
    seleccion = 0;
    setMinimumSize(640, 360);
    setFocusPolicy(Qt::StrongFocus);
    connect(&timer, &QTimer::timeout, this, &VistaJefe::paso);
#ifdef CON_MUSICA
    musica = new QMediaPlayer(this);
    salida = new QAudioOutput(this);
    salida->setVolume(0.7f);
    musica->setAudioOutput(salida);
    musica->setSource(QUrl("qrc:/musica.mp3"));
#endif
    reiniciar();
}

VistaJefe::~VistaJefe() { delete nivel; }

void VistaJefe::iniciar() {
    reloj.start();
    timer.start(16);
    setFocus();
#ifdef CON_MUSICA
    musica->play();
#endif
}

void VistaJefe::detener() {
    timer.stop();
#ifdef CON_MUSICA
    musica->pause();
#endif
}

void VistaJefe::reiniciar() {
    delete nivel;
    nivel = new NivelJefe(config);
    QFile archivo(":/MapaRitmo.txt");
    if (!archivo.open(QIODevice::ReadOnly)) throw std::runtime_error("No se encontro MapaRitmo.txt en recursos");
    std::istringstream in(archivo.readAll().toStdString());
    nivel->cargarMapaRitmo(in);

    tiempo = temblor = golpePalmera = ataqueVisual = 0;
    vidaPalmera = nivel->getPalmera().getVida();
    tipoAtaque = 0;
    disparando = terminado = pausado = false;
    seleccion = 0;
    racimos.clear();
    explosiones.clear();
    tiempoExplosiones.clear();
    pMuerte.reiniciar();
#ifdef CON_MUSICA
    musica->stop();
#endif
}

void VistaJefe::paso() {
    double dt = reloj.restart() / 1000.0;
    if (dt > 0.033) dt = 0.033;
    if (pausado) {
        update();
        return;
    }
    tiempo += dt;

    nivel->actualizar(dt);
    sincronizarMusica();
    int e;
    while (nivel->extraerEvento(e)) {
        if (e == EVENTO_DANIO_JUGADOR) temblor = 0.25;
        if (e == EVENTO_ATAQUE_JEFE) ataqueVisual = 0.5;
    }
    actualizarEfectos(dt);

    if (!terminado && nivel->getEstado() != NIVEL_EN_CURSO) {
        terminado = true;
#ifdef CON_MUSICA
        musica->stop();
#endif
    }
    update();
}

void VistaJefe::sincronizarMusica() {
#ifdef CON_MUSICA
    if (musica->playbackState() != QMediaPlayer::PlayingState || musica->position() <= 0) return;
    long long pos = musica->position();
    if (std::fabs(pos - nivel->getPosicionCancionMs()) > 80) nivel->sincronizar(pos);
#endif
}

void VistaJefe::actualizarEfectos(double dt) {
    for (Animacion* a : {&rCaminar, &mCaminar, &mAtacar, &hojas}) a->actualizar(dt);
    if (temblor > 0) temblor -= dt;
    if (ataqueVisual > 0) ataqueVisual -= dt;

    const PalmeraJefe& palmera = nivel->getPalmera();
    if (palmera.tieneAtaquePendiente()) tipoAtaque = palmera.getAtaquePendiente();
    if (palmera.getVida() < vidaPalmera) {
        if (golpePalmera <= 0) pGolpe.reiniciar();
        golpePalmera = 0.3;
    }
    vidaPalmera = palmera.getVida();
    if (golpePalmera > 0) {
        golpePalmera -= dt;
        pGolpe.actualizar(dt);
    }
    if (nivel->getEstado() == NIVEL_GANADO) pMuerte.actualizar(dt);

    std::map<const Proyectil*, QPointF> vivos;
    for (const Proyectil* p : nivel->getProyectiles())
        if (p->estaActiva() && p->getTipo() == PROYECTIL_RACIMO)
            vivos[p] = QPointF(p->getCentroX(), p->getCentroY());
    for (const auto& r : racimos)
        if (vivos.find(r.first) == vivos.end()) {
            explosiones.push_back(r.second);
            tiempoExplosiones.push_back(0.35);
        }
    racimos.swap(vivos);

    int i = 0;
    while (i < static_cast<int>(explosiones.size())) {
        tiempoExplosiones[i] -= dt;
        if (tiempoExplosiones[i] > 0) { i++; continue; }
        explosiones[i] = explosiones.back();
        explosiones.pop_back();
        tiempoExplosiones[i] = tiempoExplosiones.back();
        tiempoExplosiones.pop_back();
    }
}

void VistaJefe::paintEvent(QPaintEvent*) {
    QPainter p(this);
    p.scale(width() / 1280.0, height() / 720.0);
    p.save();
    if (temblor > 0) p.translate(std::rand() % 9 - 4, std::rand() % 9 - 4);
    p.drawPixmap(0, 0, fondo);
    dibujarRitmo(p);
    for (const std::array<double, 4>& pl : nivel->getPlataformas())
        p.drawTiledPixmap(QRectF(pl[0], pl[1], pl[2], pl[3]), plataforma);
    dibujarPalmera(p);
    dibujarMutantes(p);
    dibujarRick(p);
    dibujarProyectiles(p);
    p.restore();
    dibujarHud(p);
}

void VistaJefe::dibujarRitmo(QPainter& p) {
    int intensidad = nivel->getIntensidadActual();
    if (intensidad == 0) return;
    double fase = std::fmod(nivel->getPosicionCancionMs(), nivel->getMsPorBeat()) / nivel->getMsPorBeat();
    double pulso = std::pow(1 - fase, 3);
    QColor color = intensidad == 3 ? QColor(255, 60, 60) : intensidad == 2 ? QColor(255, 170, 60) : QColor(120, 255, 120);
    color.setAlpha(static_cast<int>(pulso * 12 * intensidad));
    p.fillRect(QRectF(0, 0, 1280, 720), color);
}

void VistaJefe::dibujarPalmera(QPainter& p) {
    const PalmeraJefe& palmera = nivel->getPalmera();
    double cx = palmera.getCentroX(), base = palmera.getY() + palmera.getAlto();

    Animacion* a = &pIdle;
    if (nivel->getEstado() == NIVEL_GANADO) a = &pMuerte;
    else if (golpePalmera > 0) a = &pGolpe;
    else if (palmera.tieneAtaquePendiente() || ataqueVisual > 0) {
        pAtaques.setCuadro(tipoAtaque);
        a = &pAtaques;
    }

    bool dormida = nivel->getIntensidadActual() == 0 && nivel->getEstado() == NIVEL_EN_CURSO;
    if (dormida) p.setOpacity(0.6);
    a->dibujar(p, cx, base, ALTO_PALMERA);
    p.setOpacity(1);

    p.setFont(QFont("Courier New", 14, QFont::Bold));
    if (dormida) {
        p.setPen(QColor(200, 220, 255));
        p.drawText(QRectF(cx - 120, base - ALTO_PALMERA - 30, 240, 24), Qt::AlignCenter, "INVULNERABLE");
    } else if (palmera.tieneAtaquePendiente() && static_cast<int>(tiempo * 10) % 2) {
        p.setPen(QColor(255, 60, 60));
        p.setFont(QFont("Courier New", 40, QFont::Bold));
        p.drawText(QRectF(cx - 60, base - ALTO_PALMERA - 50, 120, 50), Qt::AlignCenter, "!");
    }
}

void VistaJefe::dibujarRick(QPainter& p) {
    const Rick& r = nivel->getRick();
    double cx = r.getCentroX(), base = r.getY() + r.getAlto();

    if (r.getTiempoDesdePortal() < 0.35) {
        double t = r.getTiempoDesdePortal() / 0.35;
        p.setPen(QPen(QColor(90, 255, 90, static_cast<int>(255 * (1 - t))), 6));
        p.setBrush(QColor(60, 200, 60, static_cast<int>(120 * (1 - t))));
        p.drawEllipse(QPointF(cx, base - 60), 40 + 30 * t, 75 + 20 * t);
    }
    if (r.esInvulnerable() && static_cast<int>(tiempo * 20) % 2) return;

    bool espejo = r.getDireccion() < 0;
    const Animacion* a = &rIdle;
    if (r.estaAturdido() || r.esInvulnerable()) a = &rDanado;
    else if (disparando) {
        a = &rAtacar;
        espejo = false;
    } else if (std::fabs(r.getVx()) > 1) a = &rCaminar;
    a->dibujar(p, cx, base, a->getAltoCuadro() * ESC_RICK, espejo);

    if (r.estaAturdido()) {
        p.setPen(QColor(255, 240, 120));
        p.setFont(QFont("Courier New", 16, QFont::Bold));
        p.drawText(QRectF(cx - 40, base - 150, 80, 20), Qt::AlignCenter, "* * *");
    }
}

void VistaJefe::dibujarMutantes(QPainter& p) {
    for (const BananaMutante* m : nivel->getMutantes()) {
        if (!m->estaActiva()) continue;
        Animacion& a = m->estaAlerta() ? mAtacar : mCaminar;
        a.dibujar(p, m->getCentroX(), m->getY() + m->getAlto(), a.getAltoCuadro() * ESC_MUTANTE, m->getDireccion() < 0);
    }
}

void VistaJefe::dibujarProyectiles(QPainter& p) {
    for (const Proyectil* pr : nivel->getProyectiles()) {
        if (!pr->estaActiva()) continue;
        QPointF c(pr->getCentroX(), pr->getCentroY());
        int tipo = pr->getTipo();
        p.save();
        p.translate(c);
        if (tipo == PROYECTIL_RAYO_RICK) {
            p.setPen(Qt::NoPen);
            p.setBrush(QColor(120, 255, 120, 90));
            p.drawRoundedRect(QRectF(-pr->getAncho() / 2 - 6, -pr->getAlto() / 2 - 5, pr->getAncho() + 12, pr->getAlto() + 10), 8, 8);
            p.setBrush(QColor(200, 255, 200));
            p.drawRoundedRect(QRectF(-pr->getAncho() / 2, -pr->getAlto() / 2, pr->getAncho(), pr->getAlto()), 5, 5);
        } else if (tipo == PROYECTIL_BANANO) {
            p.rotate(std::atan2(pr->getVy(), pr->getVx()) * 180 / PI + 90);
            p.drawPixmap(QRectF(-14, -25, 28, 50), banano, QRectF(banano.rect()));
        } else if (tipo == PROYECTIL_RACIMO) {
            p.rotate(tiempo * 200);
            p.drawPixmap(QRectF(-30, -50, 60, 100), racimo, QRectF(racimo.rect()));
        } else if (tipo == PROYECTIL_HOJA) {
            hojas.dibujar(p, 0, 20, 40);
        } else {
            p.setPen(QPen(QColor(90, 60, 0), 1));
            p.setBrush(QColor(255, 210, 60));
            p.drawRect(QRectF(-pr->getAncho() / 2, -pr->getAlto() / 2, pr->getAncho(), pr->getAlto()));
        }
        p.restore();
    }
    for (int i = 0; i < static_cast<int>(explosiones.size()); i++) {
        p.setOpacity(tiempoExplosiones[i] / 0.35);
        p.drawPixmap(QRectF(explosiones[i].x() - 80, explosiones[i].y() - 60, 160, 96), explosion,
                     QRectF(explosion.rect()));
    }
    p.setOpacity(1);
}

void VistaJefe::dibujarHud(QPainter& p) {
    const Rick& r = nivel->getRick();
    const PalmeraJefe& palmera = nivel->getPalmera();

    p.setPen(Qt::NoPen);
    p.setBrush(QColor(0, 0, 0, 150));
    p.drawRoundedRect(QRectF(340, 12, 600, 52), 8, 8);
    p.setBrush(QColor(60, 30, 30));
    p.drawRect(QRectF(356, 40, 568, 14));
    p.setBrush(palmera.getFase() == 2 ? QColor(230, 70, 50) : QColor(110, 200, 70));
    p.drawRect(QRectF(356, 40, 568.0 * palmera.getVida() / palmera.getVidaMax(), 14));
    p.setFont(QFont("Courier New", 13, QFont::Bold));
    p.setPen(Qt::white);
    p.drawText(QRectF(356, 16, 568, 22), Qt::AlignCenter, QString("PALMERA MADRE  -  Fase %1").arg(palmera.getFase()));

    p.setPen(Qt::NoPen);
    p.setBrush(QColor(0, 0, 0, 140));
    p.drawRoundedRect(QRectF(12, 12, 300, 92), 8, 8);
    for (int i = 0; i < r.getVidaMax(); i++) {
        p.setBrush(i < r.getVida() ? QColor(230, 50, 60) : QColor(70, 40, 40));
        p.drawRoundedRect(QRectF(24 + i * 26, 22, 20, 20), 5, 5);
    }
    p.setPen(Qt::white);
    p.drawText(QPointF(24, 66), QString("Combo x%1   Puntos %2").arg(r.getMultiplicador(), 0, 'f', 1).arg(nivel->getPuntaje()));
    p.drawText(QPointF(24, 94), "Portal");
    p.setPen(Qt::NoPen);
    p.setBrush(QColor(60, 60, 60));
    p.drawRect(QRectF(100, 83, 190, 10));
    p.setBrush(r.getRecargaPortal() > 0 ? QColor(90, 160, 90) : QColor(110, 255, 110));
    p.drawRect(QRectF(100, 83, 190 * (1 - r.getRecargaPortal() / 3.0), 10));

    double avance = nivel->getPosicionCancionMs() / nivel->getDuracionMs();
    if (avance > 1) avance = 1;
    p.setBrush(QColor(0, 0, 0, 140));
    p.drawRect(QRectF(0, 712, 1280, 8));
    p.setBrush(QColor(255, 210, 90));
    p.drawRect(QRectF(0, 712, 1280 * avance, 8));

    if (pausado) {
        dibujarMenu(p, "PAUSA", QColor(255, 220, 120), QString("Puntaje: %1").arg(nivel->getPuntaje()), opciones(),
                    seleccion);
        return;
    }
    if (nivel->getEstado() == NIVEL_EN_CURSO) return;
    bool gano = nivel->getEstado() == NIVEL_GANADO;
    QString titulo = gano ? "¡LA PALMERA MADRE CAYÓ!" : !r.estaVivo() ? "RICK HA CAÍDO" : "SE ACABÓ LA CANCIÓN";
    dibujarMenu(p, titulo, gano ? QColor(255, 220, 90) : QColor(240, 80, 80),
                QString("Puntaje: %1").arg(nivel->getPuntaje()), opciones(), seleccion);
}

int VistaJefe::accionDeTecla(int tecla) const {
    switch (tecla) {
    case Qt::Key_A: case Qt::Key_Left: return ACCION_IZQUIERDA;
    case Qt::Key_D: case Qt::Key_Right: return ACCION_DERECHA;
    case Qt::Key_W: case Qt::Key_Up: case Qt::Key_Space: return ACCION_SALTAR;
    case Qt::Key_J: case Qt::Key_X: return ACCION_DISPARAR;
    case Qt::Key_K: case Qt::Key_Shift: return ACCION_PORTAL;
    }
    return -1;
}

std::vector<QString> VistaJefe::opciones() const {
    if (pausado) return {"Continuar", "Reiniciar nivel", "Salir al menú"};
    if (nivel->getEstado() == NIVEL_GANADO) return {"Continuar"};
    return {"Reintentar", "Salir al menú"};
}

void VistaJefe::pausar(bool activa) {
    pausado = activa;
    seleccion = 0;
    if (activa) {
        for (int a : {ACCION_IZQUIERDA, ACCION_DERECHA, ACCION_DISPARAR}) nivel->procesarEntrada(a, false);
        disparando = false;
#ifdef CON_MUSICA
        musica->pause();
#endif
    } else {
#ifdef CON_MUSICA
        musica->play();
#endif
    }
    setFocus();
    update();
}

void VistaJefe::elegirOpcion() {
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

void VistaJefe::keyPressEvent(QKeyEvent* e) {
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

void VistaJefe::keyReleaseEvent(QKeyEvent* e) {
    if (e->isAutoRepeat()) return;
    int a = accionDeTecla(e->key());
    if (a < 0) { QWidget::keyReleaseEvent(e); return; }
    if (a == ACCION_DISPARAR) disparando = false;
    nivel->procesarEntrada(a, false);
}
