#include "vistajefe.h"
#include "bananamutante.h"
#include "niveljefe.h"
#include "palmerajefe.h"
#include "proyectil.h"
#include "rick.h"
#include <QFile>
#include <QKeyEvent>
#include <QMouseEvent>
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

const double ESC_RICK = 1.6;
const double ESC_MUTANTE = 0.62;
const double ESC_PALMERA = 2.7;

static double perspectiva(double y) {
    double t = (y - 150) / 550;
    if (t < 0) t = 0;
    if (t > 1) t = 1;
    return 0.55 + 0.6 * t;
}

VistaJefe::VistaJefe(const ConfiguracionDificultad& config, QWidget* parent)
    : QWidget(parent), config(config), nivel(nullptr), tiempo(0), temblor(0), golpePalmera(0), ataqueVisual(0),
    tiempoFin(0), vidaPalmera(0), tipoAtaque(0), seleccion(0), disparando(false), terminado(false), pausado(false),
    mira(640, 360), fondo(":/fondo_frontal.png"), racimo(":/racimo_idle.png"), racimoBrillo(":/racimo_brillo.png"),
    rIdle(":/rick_espalda_idle.png", 2, 2), rCaminar(":/rick_espalda_caminar.png", 3, 8),
    p1Idle(":/palmera_f1_idle.png", 6, 6),
    p1Ataque(":/palmera_f1_ataque.png", 6, 12), p2Idle(":/palmera_f2_idle.png", 6, 8),
    p2Ataque(":/palmera_f2_ataque.png", 6, 12), mCaminar(":/banana_caminar.png", 3, 6),
    mAtacar(":/banana_atacar.png", 3, 10), banano(":/banana_rotar.png", 3, 10), hojas(":/efecto_hojas.png", 4, 6),
    impacto(":/efecto_impacto.png", 4, 1), onda(":/efecto_onda.png", 4, 1) {
    setMinimumSize(640, 360);
    setFocusPolicy(Qt::StrongFocus);
    setMouseTracking(true);
    setCursor(Qt::BlankCursor);
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

    tiempo = temblor = golpePalmera = ataqueVisual = tiempoFin = 0;
    vidaPalmera = nivel->getPalmera().getVida();
    tipoAtaque = seleccion = 0;
    disparando = terminado = pausado = false;
    racimos.clear();
    racimosExplotados.clear();
    posEfectos.clear();
    tipoEfectos.clear();
    tiempoEfectos.clear();
#ifdef CON_MUSICA
    musica->stop();
#endif
}

QPointF VistaJefe::aEscena(QPointF punto) const {
    return QPointF(punto.x() * 1280.0 / width(), punto.y() * 720.0 / height());
}

void VistaJefe::paso() {
    double dt = reloj.restart() / 1000.0;
    if (dt > 0.033) dt = 0.033;
    if (pausado) {
        update();
        return;
    }
    tiempo += dt;

    if (disparando) nivel->dispararHacia(mira.x(), mira.y());
    nivel->actualizar(dt);
    sincronizarMusica();
    int e;
    while (nivel->extraerEvento(e)) {
        if (e == EVENTO_DANIO_JUGADOR) temblor = 0.25;
        if (e == EVENTO_ATAQUE_JEFE) {
            ataqueVisual = 0.5;
            p1Ataque.reiniciar();
            p2Ataque.reiniciar();
            const PalmeraJefe& palmera = nivel->getPalmera();
            if (palmera.getFase() == 2) agregarEfecto(QPointF(palmera.getCentroX(), NivelJefe::Y_HORIZONTE), 2);
        }
    }
    actualizarEfectos(dt);

    if (!terminado && nivel->getEstado() != NIVEL_EN_CURSO) {
        terminado = true;
        disparando = false;
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

void VistaJefe::agregarEfecto(QPointF pos, int tipo) {
    posEfectos.push_back(pos);
    tipoEfectos.push_back(tipo);
    tiempoEfectos.push_back(0);
}

void VistaJefe::actualizarEfectos(double dt) {
    for (Animacion* a : {&rIdle, &rCaminar, &p1Idle, &p1Ataque, &p2Idle, &p2Ataque, &mCaminar, &mAtacar,
                         &banano, &hojas})
        a->actualizar(dt);
    if (temblor > 0) temblor -= dt;
    if (ataqueVisual > 0) ataqueVisual -= dt;
    if (nivel->getEstado() == NIVEL_GANADO) tiempoFin += dt;

    const PalmeraJefe& palmera = nivel->getPalmera();
    if (palmera.tieneAtaquePendiente()) tipoAtaque = palmera.getAtaquePendiente();
    if (palmera.getVida() < vidaPalmera) golpePalmera = 0.3;
    vidaPalmera = palmera.getVida();
    if (golpePalmera > 0) golpePalmera -= dt;

    std::map<const Proyectil*, QPointF> vivos;
    std::map<const Proyectil*, bool> explotados;
    for (const Proyectil* p : nivel->getProyectiles()) {
        if (!p->estaActiva() || p->getTipo() != PROYECTIL_RACIMO) continue;
        vivos[p] = QPointF(p->getCentroX(), p->getCentroY());
        explotados[p] = p->haExplotado();
        auto antes = racimosExplotados.find(p);
        if (p->haExplotado() && (antes == racimosExplotados.end() || !antes->second))
            agregarEfecto(QPointF(p->getCentroX(), p->getY() + p->getAlto()), 0);
    }
    for (const auto& r : racimos)
        if (vivos.find(r.first) == vivos.end() && !racimosExplotados[r.first]) agregarEfecto(r.second, 1);
    racimos.swap(vivos);
    racimosExplotados.swap(explotados);

    int i = 0;
    while (i < static_cast<int>(posEfectos.size())) {
        tiempoEfectos[i] += dt;
        if (tiempoEfectos[i] < 0.45) {
            i++;
            continue;
        }
        posEfectos[i] = posEfectos.back();
        posEfectos.pop_back();
        tipoEfectos[i] = tipoEfectos.back();
        tipoEfectos.pop_back();
        tiempoEfectos[i] = tiempoEfectos.back();
        tiempoEfectos.pop_back();
    }
}

void VistaJefe::paintEvent(QPaintEvent*) {
    QPainter p(this);
    p.scale(width() / 1280.0, height() / 720.0);
    p.save();
    if (temblor > 0) p.translate(std::rand() % 9 - 4, std::rand() % 9 - 4);
    p.drawPixmap(QRectF(0, 0, 1280, 720), fondo, QRectF(fondo.rect()));
    dibujarRitmo(p);
    dibujarPalmera(p);
    for (const std::array<double, 4>& pl : nivel->getPlataformas()) {
        QRectF losa(pl[0], pl[1], pl[2], pl[3]);
        p.setPen(Qt::NoPen);
        p.setBrush(QColor(0, 0, 0, 60));
        p.drawRect(losa.translated(0, losa.height()));
        p.setPen(QPen(QColor(60, 40, 40), 3));
        p.setBrush(QColor(176, 128, 108));
        p.drawRect(losa);
        p.setPen(QPen(QColor(210, 160, 130), 2));
        p.drawLine(QPointF(losa.left() + 3, losa.top() + 3), QPointF(losa.right() - 3, losa.top() + 3));
        p.setPen(QPen(QColor(110, 80, 70), 2));
        for (double x = losa.left() + 40; x < losa.right() - 10; x += 40)
            p.drawLine(QPointF(x, losa.top() + 4), QPointF(x, losa.bottom() - 2));
    }
    dibujarMutantes(p);
    dibujarProyectiles(p);
    dibujarRick(p);
    dibujarEfectos(p);
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
    bool fase2 = palmera.getFase() == 2;

    Animacion* a = fase2 ? &p2Idle : &p1Idle;
    if (nivel->getEstado() == NIVEL_EN_CURSO && (palmera.tieneAtaquePendiente() || ataqueVisual > 0))
        a = fase2 ? &p2Ataque : &p1Ataque;

    bool dormida = nivel->getIntensidadActual() == 0 && nivel->getEstado() == NIVEL_EN_CURSO;
    double opacidad = dormida ? 0.6 : 1;
    if (golpePalmera > 0 && static_cast<int>(tiempo * 30) % 2) opacidad = 0.45;
    double hundir = 0;
    if (nivel->getEstado() == NIVEL_GANADO) {
        opacidad = std::max(0.0, 1 - tiempoFin / 2.5);
        hundir = tiempoFin * 60;
    }
    p.setOpacity(opacidad);
    a->dibujar(p, cx + (tiempoFin > 0 ? std::sin(tiempoFin * 40) * 4 : 0), base + hundir,
               a->getAltoCuadro() * ESC_PALMERA);
    p.setOpacity(1);

    double tope = base - a->getAltoCuadro() * ESC_PALMERA;
    p.setFont(QFont("Courier New", 14, QFont::Bold));
    if (dormida) {
        p.setPen(QColor(200, 220, 255));
        p.drawText(QRectF(cx - 120, tope - 28, 240, 24), Qt::AlignCenter, "INVULNERABLE");
    } else if (palmera.tieneAtaquePendiente() && static_cast<int>(tiempo * 10) % 2) {
        p.setPen(QColor(255, 60, 60));
        p.setFont(QFont("Courier New", 40, QFont::Bold));
        p.drawText(QRectF(cx - 60, tope - 50, 120, 50), Qt::AlignCenter, "!");
    }
}

void VistaJefe::dibujarRick(QPainter& p) {
    const Rick& r = nivel->getRick();
    double cx = r.getCentroX(), base = r.getY() + r.getAlto();

    p.setPen(Qt::NoPen);
    p.setBrush(QColor(0, 0, 0, 70));
    p.drawEllipse(QPointF(cx, base - 4), 42, 10);
    if (r.getTiempoDesdePortal() < 0.35) {
        double t = r.getTiempoDesdePortal() / 0.35;
        p.setPen(QPen(QColor(90, 255, 90, static_cast<int>(255 * (1 - t))), 6));
        p.setBrush(QColor(60, 200, 60, static_cast<int>(120 * (1 - t))));
        p.drawEllipse(QPointF(cx, base - 65), 45 + 30 * t, 80 + 20 * t);
    }
    if (r.esInvulnerable() && static_cast<int>(tiempo * 20) % 2) return;

    const Animacion* a = std::fabs(r.getVx()) > 1 ? &rCaminar : &rIdle;
    a->dibujar(p, cx, base, a->getAltoCuadro() * ESC_RICK, a != &rIdle && r.getDireccion() < 0);
    if (r.estaAturdido()) {
        p.setPen(QColor(255, 240, 120));
        p.setFont(QFont("Courier New", 16, QFont::Bold));
        p.drawText(QRectF(cx - 40, base - a->getAltoCuadro() * ESC_RICK - 24, 80, 20), Qt::AlignCenter, "* * *");
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
        double cx = pr->getCentroX(), cy = pr->getCentroY(), s = perspectiva(cy);
        int tipo = pr->getTipo();
        if (tipo == PROYECTIL_RAYO_RICK) {
            double v = std::hypot(pr->getVx(), pr->getVy());
            QPointF cola(cx - pr->getVx() / v * 34, cy - pr->getVy() / v * 34);
            p.setPen(QPen(QColor(120, 255, 120, 110), 12, Qt::SolidLine, Qt::RoundCap));
            p.drawLine(cola, QPointF(cx, cy));
            p.setPen(QPen(QColor(225, 255, 225), 5, Qt::SolidLine, Qt::RoundCap));
            p.drawLine(cola, QPointF(cx, cy));
        } else if (tipo == PROYECTIL_RACIMO) {
            if (pr->haExplotado()) continue;
            p.setPen(Qt::NoPen);
            p.setBrush(QColor(0, 0, 0, 60));
            p.drawEllipse(QPointF(cx, NivelJefe::Y_SUELO - 6), 34 * s, 9 * s);
            const QPixmap& img = static_cast<int>(tiempo * 6) % 2 ? racimoBrillo : racimo;
            double alto = 80 * s * img.height() / racimo.height(), ancho = alto * img.width() / img.height();
            p.drawPixmap(QRectF(cx - ancho / 2, cy + 40 * s - alto, ancho, alto), img, QRectF(img.rect()));
        } else if (tipo == PROYECTIL_ONDA) {
            double r = pr->getAncho() / 2, a = pr->getAlto();
            int alfa = static_cast<int>(std::min(255.0, a * 6));
            p.setBrush(Qt::NoBrush);
            p.setPen(QPen(QColor(120, 80, 50, alfa / 2), a * 0.5));
            p.drawEllipse(QPointF(cx, NivelJefe::Y_SUELO - 8), r, r * 0.22);
            p.setPen(QPen(QColor(255, 225, 160, alfa), std::max(2.0, a * 0.18)));
            p.drawEllipse(QPointF(cx, NivelJefe::Y_SUELO - 8), r, r * 0.22);
            p.setPen(Qt::NoPen);
            p.setBrush(QColor(200, 160, 110, alfa / 2));
            for (int lado : {-1, 1}) p.drawEllipse(QPointF(cx + lado * r, NivelJefe::Y_SUELO - a / 2), 10, a / 2);
        } else if (tipo == PROYECTIL_BANANO) {
            banano.dibujar(p, cx, cy + 22 * s, 44 * s);
        } else if (tipo == PROYECTIL_HOJA) {
            hojas.dibujar(p, cx, cy + 14, 28);
        } else {
            p.setPen(QPen(QColor(90, 60, 0), 1));
            p.setBrush(QColor(255, 210, 60));
            p.drawRect(QRectF(pr->getX(), pr->getY(), pr->getAncho(), pr->getAlto()));
        }
    }
}

void VistaJefe::dibujarEfectos(QPainter& p) {
    for (int i = 0; i < static_cast<int>(posEfectos.size()); i++) {
        double t = tiempoEfectos[i] / 0.45;
        int cuadro = static_cast<int>(t * 4);
        if (cuadro > 3) cuadro = 3;
        QPointF c = posEfectos[i];
        if (tipoEfectos[i] == 0) {
            impacto.setCuadro(cuadro);
            impacto.dibujar(p, c.x(), c.y(), 80);
        } else if (tipoEfectos[i] == 1) {
            p.setPen(Qt::NoPen);
            p.setBrush(QColor(255, 240, 120, static_cast<int>(200 * (1 - t))));
            p.drawEllipse(c, 20 + 50 * t, 20 + 50 * t);
            onda.setCuadro(cuadro);
            onda.dibujar(p, c.x(), c.y() + 20, 40);
        } else {
            onda.setCuadro(cuadro);
            p.setOpacity(1 - t * 0.6);
            onda.dibujar(p, c.x(), c.y() + 10, 70);
            p.setOpacity(1);
        }
    }

    if (nivel->getEstado() != NIVEL_EN_CURSO || pausado) return;
    p.setPen(QPen(QColor(120, 255, 120), 2));
    p.setBrush(Qt::NoBrush);
    p.drawEllipse(mira, 14, 14);
    p.drawLine(QPointF(mira.x() - 22, mira.y()), QPointF(mira.x() - 8, mira.y()));
    p.drawLine(QPointF(mira.x() + 8, mira.y()), QPointF(mira.x() + 22, mira.y()));
    p.drawLine(QPointF(mira.x(), mira.y() - 22), QPointF(mira.x(), mira.y() - 8));
    p.drawLine(QPointF(mira.x(), mira.y() + 8), QPointF(mira.x(), mira.y() + 22));
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
    case Qt::Key_K: case Qt::Key_Shift: return ACCION_PORTAL;
    }
    return -1;
}

void VistaJefe::mousePressEvent(QMouseEvent* e) {
    mira = aEscena(e->position());
    if (e->button() != Qt::LeftButton || pausado || nivel->getEstado() != NIVEL_EN_CURSO) {
        QWidget::mousePressEvent(e);
        return;
    }
    disparando = true;
    nivel->dispararHacia(mira.x(), mira.y());
}

void VistaJefe::mouseReleaseEvent(QMouseEvent* e) {
    if (e->button() == Qt::LeftButton) disparando = false;
}

void VistaJefe::mouseMoveEvent(QMouseEvent* e) { mira = aEscena(e->position()); }

std::vector<QString> VistaJefe::opciones() const {
    if (pausado) return {"Continuar", "Reiniciar nivel", "Salir al menú"};
    if (nivel->getEstado() == NIVEL_GANADO) return {"Continuar"};
    return {"Reintentar", "Salir al menú"};
}

void VistaJefe::pausar(bool activa) {
    pausado = activa;
    seleccion = 0;
    if (activa) {
        for (int a : {ACCION_IZQUIERDA, ACCION_DERECHA}) nivel->procesarEntrada(a, false);
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
    nivel->procesarEntrada(a, true);
}

void VistaJefe::keyReleaseEvent(QKeyEvent* e) {
    if (e->isAutoRepeat()) return;
    int a = accionDeTecla(e->key());
    if (a < 0) { QWidget::keyReleaseEvent(e); return; }
    nivel->procesarEntrada(a, false);
}
