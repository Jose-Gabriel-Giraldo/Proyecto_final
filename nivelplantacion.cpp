#include "nivelplantacion.h"
#include "bananamutante.h"
#include "fisicapendular.h"
#include "morty.h"
#include "proyectil.h"
#include <cmath>

const double NivelPlantacion::ANCHO_MUNDO = 6400;
const double NivelPlantacion::ALTO_MUNDO = 720;
const double NivelPlantacion::Y_SUELO = 640;
const double NivelPlantacion::AGUA_INICIO = 2650;
const double NivelPlantacion::AGUA_FIN = 3850;
const double NivelPlantacion::X_ESTACION = 5700;
const double NivelPlantacion::TIEMPO_JAULA = 1.5;

NivelPlantacion::NivelPlantacion()
    : morty(new Morty(100, Y_SUELO - 90)), checkpointX(100), izquierda(false), derecha(false), disparando(false),
    interactuar(false) {
    xJaulas = {700, 2000, 3950, 4650, 5150};
    progresoJaulas.assign(xJaulas.size(), 0.0);

    bananas.push_back(new BananaMutante(450, Y_SUELO - 110, 300, 1100));
    bananas.push_back(new BananaMutante(1500, Y_SUELO - 110, 1250, 1750));
    bananas.push_back(new BananaMutante(2300, Y_SUELO - 110, 2100, 2600));
    bananas.push_back(new BananaMutante(4100, Y_SUELO - 110, 3950, 4450));

    for (double x : {4400.0, 4950.0, 5300.0}) bananas.push_back(new BananaMutante(x, Y_SUELO - 110, x, x + 70, true, 0, 500, 3));

    for (double x : {2850.0, 3200.0, 3550.0}) lianas.push_back(new FisicaPendular(x, 100, 400, 0.0, 0.6));

    plataformas = {{1350, 500, 260, 22}, {1700, 380, 220, 22}, {2150, 480, 260, 22},
                   {4300, 500, 300, 22}, {4750, 400, 250, 22}};
}

NivelPlantacion::~NivelPlantacion() {
    delete morty;
    for (BananaMutante* b : bananas) delete b;
    for (FisicaPendular* l : lianas) delete l;
}

void NivelPlantacion::actualizar(double dt) {
    if (estado != NIVEL_EN_CURSO) return;
    tiempo += dt;

    for (FisicaPendular* l : lianas) l->avanzar(dt);

    int cerca = 0;
    for (BananaMutante* b : bananas)
        if (b->estaActiva() && std::fabs(b->getCentroX() - morty->getCentroX()) < 300) cerca++;
    morty->setEnemigosCerca(cerca);
    morty->mover(static_cast<int>(derecha) - static_cast<int>(izquierda));

    if (disparando) {
        Proyectil* p = morty->disparar();
        if (p) { agregarProyectil(p); eventos.push(EVENTO_DISPARO); }
    }

    double piesAntes = morty->getY() + morty->getAlto();
    morty->actualizar(dt);
    resolverTerreno(piesAntes);
    resolverLiberacion(dt);

    for (BananaMutante* b : bananas)
        if (b->estaActiva()) {
            b->comportarse(*morty, dt);
            b->actualizar(dt);
            agregarProyectil(b->disparar());
        }

    actualizarProyectiles(dt);
    resolverCombate();
    limpiarProyectiles();

    if (!morty->estaVivo()) estado = NIVEL_PERDIDO;
    else if (morty->getX() >= X_ESTACION && morty->getLiberados() >= MINIMO_LIBERADOS) {
        estado = NIVEL_GANADO;
        puntaje += morty->getVida() * 200;
    }
}

void NivelPlantacion::resolverTerreno(double piesAntes) {
    if (morty->getLiana()) return;
    double pies = morty->getY() + morty->getAlto();

    if (morty->getVy() >= 0)
        for (const auto& p : plataformas)
            if (piesAntes <= p[1] + 0.5 && pies >= p[1] && morty->getX() + morty->getAncho() > p[0] &&
                morty->getX() < p[0] + p[2]) {
                morty->aterrizar(p[1]);
                break;
            }

    double cx = morty->getCentroX();
    bool sobreAgua = cx > AGUA_INICIO && cx < AGUA_FIN;
    if (!sobreAgua && morty->getY() + morty->getAlto() >= Y_SUELO) morty->aterrizar(Y_SUELO);

    if (morty->estaEnSuelo() && !sobreAgua)
        for (double c : {100.0, 1200.0, 2450.0, 3950.0, 5400.0})
            if (morty->getX() >= c && c > checkpointX) checkpointX = c;

    if (sobreAgua && morty->getY() > Y_SUELO + 150) {
        morty->perderVida();
        morty->setPosicion(checkpointX, Y_SUELO - morty->getAlto());
        morty->setVelocidad(0, 0);
        morty->aterrizar(Y_SUELO);
        eventos.push(EVENTO_DANIO_JUGADOR);
    }

    double x = morty->getX();
    if (x < 0) x = 0;
    if (x > ANCHO_MUNDO - morty->getAncho()) x = ANCHO_MUNDO - morty->getAncho();
    morty->setPosicion(x, morty->getY());
}

void NivelPlantacion::resolverLiberacion(double dt) {
    if (interactuar && !morty->getLiana() && morty->estaEnSuelo())
        for (size_t i = 0; i < xJaulas.size(); i++)
            if (!jaulaLiberada(static_cast<int>(i)) && std::fabs(xJaulas[i] + 30 - morty->getCentroX()) < 70) {
                morty->setLiberando(true);
                progresoJaulas[i] += dt;
                if (jaulaLiberada(static_cast<int>(i))) {
                    morty->registrarLiberado();
                    puntaje += 500;
                    eventos.push(EVENTO_TRABAJADOR_LIBERADO);
                }
                return;
            }
    morty->setLiberando(false);
}

void NivelPlantacion::resolverCombate() {
    int vidaAntes = morty->getVida();
    for (Proyectil* p : proyectiles) {
        if (!p->estaActiva()) continue;
        if (!p->esDelJugador()) {
            if (p->colisionaCon(*morty)) {
                morty->recibirDanio(p->getDanio());
                p->desactivar();
            }
            continue;
        }
        for (BananaMutante* b : bananas)
            if (p->colisionaCon(*b)) {
                b->recibirDanio(p->getDanio());
                p->desactivar();
                if (!b->estaVivo()) {
                    b->desactivar();
                    puntaje += b->esArmado() ? 150 : 100;
                    eventos.push(EVENTO_ENEMIGO_DERROTADO);
                }
                break;
            }
    }
    for (BananaMutante* b : bananas)
        if (b->colisionaCon(*morty)) morty->recibirDanio(1);
    if (morty->getVida() < vidaAntes) eventos.push(EVENTO_DANIO_JUGADOR);
}

void NivelPlantacion::procesarEntrada(int a, bool presionada) {
    switch (a) {
    case ACCION_IZQUIERDA: izquierda = presionada; break;
    case ACCION_DERECHA: derecha = presionada; break;
    case ACCION_CORRER: morty->setCorrer(presionada); break;
    case ACCION_DISPARAR: disparando = presionada; break;
    case ACCION_SALTAR:
        if (presionada) morty->saltar();
        break;
    case ACCION_INTERACTUAR:
        interactuar = presionada;
        if (!presionada) break;
        if (morty->getLiana()) {
            morty->soltarLiana();
            interactuar = false;
            break;
        }
        for (FisicaPendular* l : lianas)
            if (std::hypot(l->getPuntoX() - morty->getCentroX(), l->getPuntoY() - morty->getCentroY()) < 80) {
                morty->agarrarLiana(l);
                interactuar = false;
                break;
            }
        break;
    case ACCION_PORTAL: break;
    }
}

const Morty& NivelPlantacion::getMorty() const { return *morty; }
const std::vector<double>& NivelPlantacion::getXJaulas() const { return xJaulas; }
const std::vector<double>& NivelPlantacion::getProgresoJaulas() const { return progresoJaulas; }
bool NivelPlantacion::jaulaLiberada(int i) const { return progresoJaulas[i] >= TIEMPO_JAULA; }
const std::vector<BananaMutante*>& NivelPlantacion::getBananas() const { return bananas; }
const std::vector<FisicaPendular*>& NivelPlantacion::getLianas() const { return lianas; }
const std::vector<std::array<double, 4>>& NivelPlantacion::getPlataformas() const { return plataformas; }

bool NivelPlantacion::faltanTrabajadores() const {
    return morty->getX() >= X_ESTACION && morty->getLiberados() < MINIMO_LIBERADOS;
}
