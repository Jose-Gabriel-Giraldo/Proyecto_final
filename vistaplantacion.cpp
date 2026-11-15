#include "vistaplantacion.h"
#include "bananamutante.h"
#include "fisicapendular.h"
#include "morty.h"
#include "nivelplantacion.h"
#include "proyectil.h"
#include "tipo.h"
#include <QFont>
#include <QPainter>
#include <QPen>
#include <cmath>

const int ANCHO_VENTANA = 1280;
const int ALTO_VENTANA = 720;
const int MS_POR_CUADRO = 16;          // ~60 cuadros por segundo
const double DT = MS_POR_CUADRO / 1000.0;

VistaPlantacion::VistaPlantacion(QWidget* parent)
    : QWidget(parent),
    cielo(cargarImagen(":/recursos/cielo.png")),
    montanas(cargarImagen(":/recursos/montanas.png")),
    fondoNivel(cargarImagen(":/recursos/nivel1.png")),
    plataforma(cargarImagen(":/recursos/plataforma.png")),
    mortyQuieto(":/recursos/morty_idle.png", 1),
    mortyCaminar(":/recursos/morty_caminar.png", 3),
    mortyCorrer(":/recursos/morty_correr.png", 3),
    mortyAtacar(":/recursos/morty_atacar.png", 1),
    mortyDanado(":/recursos/morty_danado.png", 1),
    bananaCaminar(":/recursos/banana_caminar.png", 3),
    bananaAtacar(":/recursos/banana_atacar.png", 3),
    soldadoQuieto(":/recursos/soldado_idle.png", 4),
    soldadoDisparar(":/recursos/soldado_disparar.png", 5),
    jaulaQuieta(":/recursos/jaula_idle.png", 4),
    jaulaLiberar(":/recursos/jaula_liberar.png", 4),
    nivel(nullptr), temporizador(new QTimer(this)), camaraX(0), cuadroAnimacion(0), pausado(false),
    disparando(false), avisoEmitido(false), temblor(0) {
    // El nivel se crea al final: si alguna imagen fallo, ya se lanzo la excepcion
    // y no queda memoria reservada sin liberar.
    nivel = new NivelPlantacion();

    setFixedSize(ANCHO_VENTANA, ALTO_VENTANA);
    setFocusPolicy(Qt::StrongFocus);   // para recibir las teclas

    // Conexion senal-ranura: cada vez que el temporizador "suena" se llama actualizarJuego
    connect(temporizador, &QTimer::timeout, this, &VistaPlantacion::actualizarJuego);
    temporizador->start(MS_POR_CUADRO);
}

VistaPlantacion::~VistaPlantacion() { delete nivel; }

// ---------------------------------------------------------------- ciclo del juego

void VistaPlantacion::actualizarJuego() {
    cuadroAnimacion++;
    if (temblor > 0) temblor--;

    if (!pausado) {
        nivel->actualizar(DT);
        leerEventos();
        moverCamara();
    }

    if (nivel->getEstado() != NIVEL_EN_CURSO && !avisoEmitido) {
        avisoEmitido = true;
        emit nivelTerminado(nivel->getEstado() == NIVEL_GANADO, nivel->getPuntaje());
    }
    update();   // pide a Qt que vuelva a llamar a paintEvent
}

void VistaPlantacion::leerEventos() {
    // El nivel deja en una cola lo que paso en este cuadro.
    // Aqui se conectaran los sonidos; por ahora el dano hace temblar la camara.
    int evento;
    while (nivel->extraerEvento(evento)) {
        if (evento == EVENTO_DANIO_JUGADOR) temblor = 12;
    }
}

void VistaPlantacion::moverCamara() {
    // La camara sigue a Morty pero nunca muestra fuera del mapa
    camaraX = nivel->getMorty().getCentroX() - ANCHO_VENTANA / 2.0;
    double maximo = NivelPlantacion::ANCHO_MUNDO - ANCHO_VENTANA;
    if (camaraX < 0) camaraX = 0;
    if (camaraX > maximo) camaraX = maximo;
}

void VistaPlantacion::reiniciar() {
    delete nivel;                    // se libera el nivel anterior...
    nivel = new NivelPlantacion();   // ...y se crea uno nuevo desde cero
    camaraX = 0;
    pausado = false;
    disparando = false;
    avisoEmitido = false;
    temblor = 0;
}

// ---------------------------------------------------------------- teclado

int VistaPlantacion::teclaAAccion(int tecla) const {
    switch (tecla) {
    case Qt::Key_A:
    case Qt::Key_Left: return ACCION_IZQUIERDA;
    case Qt::Key_D:
    case Qt::Key_Right: return ACCION_DERECHA;
    case Qt::Key_W:
    case Qt::Key_Up:
    case Qt::Key_Space: return ACCION_SALTAR;
    case Qt::Key_Shift: return ACCION_CORRER;
    case Qt::Key_J: return ACCION_DISPARAR;
    case Qt::Key_E: return ACCION_INTERACTUAR;
    default: return -1;
    }
}

void VistaPlantacion::keyPressEvent(QKeyEvent* evento) {
    // Al dejar una tecla presionada el sistema repite el evento; solo nos interesa la primera vez
    if (evento->isAutoRepeat()) return;

    if (evento->key() == Qt::Key_Escape && nivel->getEstado() == NIVEL_EN_CURSO) {
        pausado = !pausado;
        return;
    }
    if ((evento->key() == Qt::Key_Return || evento->key() == Qt::Key_Enter) && nivel->getEstado() != NIVEL_EN_CURSO) {
        reiniciar();
        return;
    }

    int accion = teclaAAccion(evento->key());
    if (accion < 0) {
        QWidget::keyPressEvent(evento);
        return;
    }
    if (pausado) return;
    if (accion == ACCION_DISPARAR) disparando = true;
    nivel->procesarEntrada(accion, true);
}

void VistaPlantacion::keyReleaseEvent(QKeyEvent* evento) {
    if (evento->isAutoRepeat()) return;
    int accion = teclaAAccion(evento->key());
    if (accion < 0) {
        QWidget::keyReleaseEvent(evento);
        return;
    }
    if (accion == ACCION_DISPARAR) disparando = false;
    nivel->procesarEntrada(accion, false);
}

// ---------------------------------------------------------------- dibujo

void VistaPlantacion::paintEvent(QPaintEvent*) {
    QPainter p(this);
    dibujarFondo(p);

    // Todo lo del mundo se dibuja desplazado por la camara
    p.save();
    double sacudida = temblor > 0 ? (temblor % 2 == 0 ? 5 : -5) : 0;
    p.translate(-camaraX + sacudida, 0);
    dibujarEscenario(p);
    dibujarEnemigos(p);
    dibujarProyectiles(p);
    dibujarMorty(p);
    p.restore();

    dibujarHud(p);

    if (nivel->getEstado() == NIVEL_GANADO)
        dibujarMensaje(p, "¡Llegaste a la estación! Puntaje: " + QString::number(nivel->getPuntaje()) +
                              "\n(Enter para jugar de nuevo)");
    else if (nivel->getEstado() == NIVEL_PERDIDO)
        dibujarMensaje(p, "Morty se quedó sin vidas...\n(Enter para reintentar)");
    else if (pausado)
        dibujarMensaje(p, "PAUSA\n(Esc para continuar)");
    else if (nivel->faltanTrabajadores())
        dibujarMensaje(p, "RICK: ¡Aún faltan trabajadores, Morty!\nNecesitas liberar al menos " +
                              QString::number(NivelPlantacion::MINIMO_LIBERADOS) + ".");
}

void VistaPlantacion::dibujarFondo(QPainter& p) {
    p.drawPixmap(0, 0, cielo);
    // Paralaje: las montanas se mueven mas lento que el nivel (30 %), y se repiten
    double desfase = std::fmod(camaraX * 0.3, montanas.width());
    p.drawPixmap(QPointF(-desfase, 0), montanas);
    p.drawPixmap(QPointF(-desfase + montanas.width(), 0), montanas);
}

void VistaPlantacion::dibujarEscenario(QPainter& p) {
    p.drawPixmap(0, 0, fondoNivel);   // la imagen mide lo mismo que el mundo (6400 x 720)

    // Plataformas: cada una es {x, y, ancho, alto}; la textura se repite a lo ancho
    for (const auto& pl : nivel->getPlataformas())
        p.drawTiledPixmap(QRectF(pl[0], pl[1], pl[2], pl[3]), plataforma);

    // Lianas: una cuerda desde el ancla hasta el punto que calcula la fisica pendular
    p.setPen(QPen(QColor(70, 110, 30), 6, Qt::SolidLine, Qt::RoundCap));
    for (const FisicaPendular* l : nivel->getLianas()) {
        p.drawLine(QPointF(l->getXAncla(), l->getYAncla()), QPointF(l->getPuntoX(), l->getPuntoY()));
        p.setBrush(QColor(40, 140, 40));
        p.drawEllipse(QPointF(l->getPuntoX(), l->getPuntoY()), 9, 9);
    }
    p.setPen(Qt::NoPen);

    // Jaulas: quieta, abriendose (segun el progreso) o abierta
    const std::vector<double>& xs = nivel->getXJaulas();
    const std::vector<double>& progreso = nivel->getProgresoJaulas();
    for (size_t i = 0; i < xs.size(); i++) {
        double centro = xs[i] + 30;
        if (nivel->jaulaLiberada(static_cast<int>(i))) {
            jaulaLiberar.dibujar(p, 3, centro, NivelPlantacion::Y_SUELO, 120, false);
        } else if (progreso[i] > 0) {
            int c = static_cast<int>(progreso[i] / NivelPlantacion::TIEMPO_JAULA * 3);
            jaulaLiberar.dibujar(p, c, centro, NivelPlantacion::Y_SUELO, 120, false);
            // barra de progreso encima de la jaula
            p.setBrush(QColor(0, 0, 0, 150));
            p.drawRect(QRectF(centro - 40, NivelPlantacion::Y_SUELO - 140, 80, 10));
            p.setBrush(QColor(80, 220, 80));
            p.drawRect(QRectF(centro - 40, NivelPlantacion::Y_SUELO - 140, 80 * progreso[i] / NivelPlantacion::TIEMPO_JAULA, 10));
        } else {
            jaulaQuieta.dibujar(p, cuadroAnimacion / 12, centro, NivelPlantacion::Y_SUELO, 120, false);
        }
    }
}

void VistaPlantacion::dibujarEnemigos(QPainter& p) {
    int cuadro = cuadroAnimacion / 8;
    for (const BananaMutante* b : nivel->getBananas()) {
        if (!b->estaActiva()) continue;
        double pies = b->getY() + b->getAlto();
        bool espejo = b->getDireccion() < 0;   // los sprites originales miran a la derecha
        if (b->esArmado()) {
            const HojaSprites& hoja = b->estaAlerta() ? soldadoDisparar : soldadoQuieto;
            hoja.dibujar(p, cuadro, b->getCentroX(), pies, 120, espejo);
        } else {
            const HojaSprites& hoja = b->estaAlerta() ? bananaAtacar : bananaCaminar;
            hoja.dibujar(p, cuadro, b->getCentroX(), pies, 115, espejo);
        }
    }
}

void VistaPlantacion::dibujarMorty(QPainter& p) {
    const Morty& m = nivel->getMorty();
    // Parpadeo mientras es invulnerable despues de un golpe
    if (m.esInvulnerable() && (cuadroAnimacion / 4) % 2 == 0) return;

    // Se escoge la animacion segun lo que esta haciendo
    const HojaSprites* hoja = &mortyQuieto;
    if (m.esInvulnerable()) hoja = &mortyDanado;
    else if (disparando && !m.estaLiberando() && !m.getLiana()) hoja = &mortyAtacar;
    else if (m.estaEnSuelo() && std::fabs(m.getVx()) > 300) hoja = &mortyCorrer;
    else if (m.estaEnSuelo() && std::fabs(m.getVx()) > 1) hoja = &mortyCaminar;

    hoja->dibujar(p, cuadroAnimacion / 8, m.getCentroX(), m.getY() + m.getAlto(), 95, m.getDireccion() < 0);

    // Aura dorada cuando Morty esta en racha de valentia
    if (m.estaEnValentia()) {
        p.setBrush(Qt::NoBrush);
        p.setPen(QPen(QColor(255, 200, 0, 160), 3));
        p.drawEllipse(QPointF(m.getCentroX(), m.getCentroY()), 45, 60);
        p.setPen(Qt::NoPen);
    }
}

void VistaPlantacion::dibujarProyectiles(QPainter& p) {
    for (const Proyectil* pr : nivel->getProyectiles()) {
        if (!pr->estaActiva()) continue;
        QRectF r(pr->getX(), pr->getY(), pr->getAncho(), pr->getAlto());
        if (pr->getTipo() == PROYECTIL_DISPARO_MORTY) p.setBrush(QColor(120, 255, 90));   // laser verde
        else p.setBrush(QColor(255, 210, 60));                                            // bala del soldado
        p.drawRoundedRect(r, 3, 3);
    }
}

void VistaPlantacion::dibujarHud(QPainter& p) {
    const Morty& m = nivel->getMorty();
    p.setBrush(QColor(0, 0, 0, 150));
    p.drawRoundedRect(QRectF(10, 10, 330, 118), 8, 8);

    p.setPen(Qt::white);
    p.setFont(QFont("Arial", 13, QFont::Bold));
    p.drawText(25, 36, "Vidas: " + QString::number(m.getVida()) + " / " + QString::number(m.getVidaMax()));
    p.drawText(25, 60, "Trabajadores: " + QString::number(m.getLiberados()) + " / " +
                           QString::number(nivel->getXJaulas().size()) + "  (mínimo " +
                           QString::number(NivelPlantacion::MINIMO_LIBERADOS) + ")");
    p.drawText(25, 84, "Puntaje: " + QString::number(nivel->getPuntaje()) + "    Tiempo: " +
                           QString::number(static_cast<int>(nivel->getTiempo())) + " s");

    // Barra de valor: al llenarse Morty entra en valentia; despues viene el cansancio
    QString estado = m.estaEnValentia() ? "¡VALENTÍA!" : (m.estaCansado() ? "Cansado" : "Valor");
    p.drawText(25, 112, estado);
    p.setPen(Qt::NoPen);
    p.setBrush(QColor(60, 60, 60));
    p.drawRect(QRectF(130, 100, 190, 14));
    double relleno = m.estaEnValentia() ? 1.0 : m.getValor() / 100.0;
    p.setBrush(m.estaEnValentia() ? QColor(255, 200, 0) : QColor(230, 140, 40));
    p.drawRect(QRectF(130, 100, 190 * relleno, 14));
}

void VistaPlantacion::dibujarMensaje(QPainter& p, const QString& texto) {
    QRectF caja(ANCHO_VENTANA / 2.0 - 330, ALTO_VENTANA / 2.0 - 70, 660, 140);
    p.setPen(Qt::NoPen);
    p.setBrush(QColor(0, 0, 0, 190));
    p.drawRoundedRect(caja, 12, 12);
    p.setPen(Qt::white);
    p.setFont(QFont("Arial", 18, QFont::Bold));
    p.drawText(caja, Qt::AlignCenter, texto);
}
