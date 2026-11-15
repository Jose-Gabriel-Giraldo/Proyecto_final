#include "nivelplantacion.h"
#include "bananamutante.h"
#include "fisicapendular.h"
#include "morty.h"
#include "proyectil.h"
#include <cmath>

const double NivelPlantacion::ANCHO_MUNDO = 15720;
const double NivelPlantacion::ALTO_MUNDO = 720;
const double NivelPlantacion::Y_SUELO = 640;
const double NivelPlantacion::AGUA_INICIO = 2650;
const double NivelPlantacion::AGUA_FIN = 3850;
const double NivelPlantacion::X_ESTACION = 15020;
const double NivelPlantacion::TIEMPO_JAULA = 1.5;

NivelPlantacion::NivelPlantacion()
    : morty(new Morty(100, Y_SUELO - 90)), checkpointX(100), izquierda(false), derecha(false), disparando(false),
    interactuar(false) {
    xJaulas = {700, 2000, 3950, 4650, 5150, 7650, 9680, 11070, 14020};
    yJaulas = {Y_SUELO, Y_SUELO, Y_SUELO, Y_SUELO, Y_SUELO, Y_SUELO, 170, 560, 370};
    progresoJaulas.assign(xJaulas.size(), 0.0);

    huecos = {{AGUA_INICIO, AGUA_FIN}, {6000, 7200}, {10350, 11550}, {12350, 13550}};
    checkpoints = {100, 1200, 2450, 3950, 5400, 5800, 7350, 8050, 10100, 11650, 12100, 13650, 14300};

    plataformas = {{1350, 500, 260, 22},  {1700, 380, 220, 22},  {2150, 480, 260, 22}, {4300, 500, 300, 22},
                   {4750, 400, 250, 22},  {7350, 470, 260, 22},  {8050, 500, 200, 22}, {8300, 370, 200, 22},
                   {8600, 250, 500, 22},  {9300, 250, 180, 22},  {9600, 170, 220, 22}, {11030, 560, 180, 22},
                   {13700, 500, 180, 22}, {13950, 370, 200, 22}};

    double suelo = Y_SUELO - 110;
    bananas.push_back(new BananaMutante(450, suelo, 300, 1100));
    bananas.push_back(new BananaMutante(1500, suelo, 1250, 1750));
    bananas.push_back(new BananaMutante(2300, suelo, 2100, 2600));
    bananas.push_back(new BananaMutante(4100, suelo, 3950, 4450));
    bananas.push_back(new BananaMutante(5700, suelo, 5550, 5950));
    bananas.push_back(new BananaMutante(7500, suelo, 7250, 8000));
    bananas.push_back(new BananaMutante(8400, suelo, 8000, 9050));
    bananas.push_back(new BananaMutante(8700, 250 - 110, 8600, 8950));
    bananas.push_back(new BananaMutante(9400, suelo, 9100, 10250));
    bananas.push_back(new BananaMutante(11900, suelo, 11600, 12300));
    bananas.push_back(new BananaMutante(14200, suelo, 13600, 14800));

    const double soldados[][2] = {{1780, 270},   {2200, 370},  {4400, suelo},  {4950, suelo},  {5300, suelo},
                                  {7450, 360},   {7950, suelo}, {9010, 140},   {9950, suelo},  {11700, suelo},
                                  {13650, suelo}, {14030, 260}, {14400, suelo}, {14650, suelo}};
    for (const auto& s : soldados) bananas.push_back(new BananaMutante(s[0], s[1], s[0], s[0] + 70, true, 0, 750, 4));

    for (double x : {2850.0, 3200.0, 3550.0, 6200.0, 6550.0, 6900.0, 10550.0, 10900.0, 11250.0, 12550.0, 12900.0,
                     13250.0})
        lianas.push_back(new FisicaPendular(x, 100, 400, 0.0, 0.6));
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

    bool hueco = sobreHueco(morty->getCentroX());
    if (!hueco && morty->getY() + morty->getAlto() >= Y_SUELO) morty->aterrizar(Y_SUELO);

    if (morty->estaEnSuelo() && !hueco)
        for (double c : checkpoints)
            if (morty->getX() >= c && c > checkpointX) checkpointX = c;

    if (hueco && morty->getY() > Y_SUELO + 150) {
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
            if (!jaulaLiberada(static_cast<int>(i)) && std::fabs(xJaulas[i] + 30 - morty->getCentroX()) < 70 &&
                std::fabs(morty->getY() + morty->getAlto() - yJaulas[i]) < 1) {
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
const std::vector<double>& NivelPlantacion::getYJaulas() const { return yJaulas; }
const std::vector<std::array<double, 2>>& NivelPlantacion::getHuecos() const { return huecos; }

bool NivelPlantacion::sobreHueco(double x) const {
    for (const auto& h : huecos)
        if (x > h[0] && x < h[1]) return true;
    return false;
}

const std::vector<double>& NivelPlantacion::getProgresoJaulas() const { return progresoJaulas; }
bool NivelPlantacion::jaulaLiberada(int i) const { return progresoJaulas[i] >= TIEMPO_JAULA; }
const std::vector<BananaMutante*>& NivelPlantacion::getBananas() const { return bananas; }
const std::vector<FisicaPendular*>& NivelPlantacion::getLianas() const { return lianas; }
const std::vector<std::array<double, 4>>& NivelPlantacion::getPlataformas() const { return plataformas; }

bool NivelPlantacion::faltanTrabajadores() const {
    return morty->getX() >= X_ESTACION && morty->getLiberados() < MINIMO_LIBERADOS;
}
