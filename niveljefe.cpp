#include "niveljefe.h"
#include "bananamutante.h"
#include "fisicaonda.h"
#include "moduloaccion.h"
#include "palmerajefe.h"
#include "proyectil.h"
#include "rick.h"
#include <cmath>
#include <istream>
#include <string>
#include <stdexcept>

const double NivelJefe::ANCHO_PANTALLA = 1280;
const double NivelJefe::ALTO_PANTALLA = 720;
const double NivelJefe::Y_SUELO = 700;
const double NivelJefe::Y_HORIZONTE = 470;

NivelJefe::NivelJefe(const ConfiguracionDificultad& config, unsigned semilla)
    : config(config), rick(new Rick(610, Y_SUELO - 120, 60, 1160)),
    palmera(new PalmeraJefe(520, Y_HORIZONTE - 330, *rick, this->config, semilla)), msPorBeat(500), posCancionMs(0),
    proximoContratiempo(-1), duracionMs(0), contadorBeats(0), idAtaqueMutantes(-1), intensidadActual(0), izquierda(false),
    derecha(false), generador(semilla + 2) {
    rick->aterrizar(Y_SUELO);
    plataformas = {{200, 560, 260, 24}, {820, 560, 260, 24}};
}

NivelJefe::~NivelJefe() {
    delete palmera;
    delete rick;
    for (BananaMutante* m : mutantes) delete m;
}

void NivelJefe::cargarMapaRitmo(std::istream& in) {
    double bpm;
    long long duracion;
    if (!(in >> bpm >> duracion) || bpm <= 0 || duracion <= 0)
        throw std::runtime_error("Encabezado invalido en el mapa de ritmo");

    std::queue<long long> tiempos;
    std::queue<int> niveles;
    long long t, anterior = -1;
    int intensidad;
    while (in >> t) {
        if (!(in >> intensidad)) throw std::runtime_error("Falta la intensidad del beat " + std::to_string(t));
        if (t < 0 || t <= anterior || t > duracion)
            throw std::runtime_error("Tiempo de beat invalido: " + std::to_string(t));
        if (intensidad < 0 || intensidad > 3)
            throw std::runtime_error("Intensidad invalida en el beat " + std::to_string(t));
        tiempos.push(t);
        niveles.push(intensidad);
        anterior = t;
    }
    if (!in.eof()) throw std::runtime_error("Dato no numerico en el mapa de ritmo");
    if (tiempos.empty()) throw std::runtime_error("El mapa de ritmo no tiene beats");

    beats.swap(tiempos);
    intensidades.swap(niveles);
    msPorBeat = 60000.0 / bpm;
    duracionMs = duracion;
}

void NivelJefe::sincronizar(long long posMs) { posCancionMs = static_cast<double>(posMs); }

void NivelJefe::actualizar(double dt) {
    if (estado != NIVEL_EN_CURSO) return;
    if (duracionMs <= 0) throw std::runtime_error("No se ha cargado el mapa de ritmo");
    tiempo += dt;
    posCancionMs += dt * 1000.0;

    rick->mover(static_cast<int>(derecha) - static_cast<int>(izquierda));
    double piesAntes = rick->getY() + rick->getAlto();
    rick->actualizar(dt);
    double pies = rick->getY() + rick->getAlto();
    if (rick->getVy() >= 0)
        for (const auto& p : plataformas)
            if (piesAntes <= p[1] + 0.5 && pies >= p[1] && rick->getX() + rick->getAncho() > p[0] &&
                rick->getX() < p[0] + p[2]) {
                rick->aterrizar(p[1]);
                break;
            }
    if (rick->getY() + rick->getAlto() >= Y_SUELO) rick->aterrizar(Y_SUELO);

    palmera->actualizar(dt);
    procesarRitmo();

    for (BananaMutante* m : mutantes) {
        m->comportarse(*rick, dt);
        m->actualizar(dt);
    }

    actualizarProyectiles(dt);
    resolverColisiones();
    limpiarProyectiles();

    if (!palmera->estaVivo()) {
        estado = NIVEL_GANADO;
        puntaje += rick->getVida() * 500;
    } else if (!rick->estaVivo() || posCancionMs >= duracionMs) {
        estado = NIVEL_PERDIDO;
    }
}

void NivelJefe::procesarRitmo() {
    while (!beats.empty() && beats.front() - config.getAvisoMs() <= posCancionMs) {
        long long b = beats.front();
        intensidadActual = intensidades.front();
        beats.pop();
        intensidades.pop();
        contadorBeats++;
        if (intensidadActual == 0) continue;
        if (contadorBeats % (palmera->getFase() == 1 ? 4 : 3) == 0)
            agregarProyectil(ModuloAccion::crearRacimo(palmera->getXBoca(), palmera->getYBoca(), rick->getCentroX(), Y_SUELO,
                                                       config.getTiempoVueloRacimo(), -1));
        int cada = (palmera->getFase() == 1 ? 16 : 8) >> intensidadActual;
        if (contadorBeats % cada == 0 && !palmera->tieneAtaquePendiente())
            palmera->preparar(b, false, static_cast<int>(mutantes.size()));
    }
    if (palmera->tieneAtaquePendiente() && posCancionMs >= palmera->getTiempoPendiente()) lanzarAtaque(true);

    if (proximoContratiempo >= 0 && posCancionMs >= proximoContratiempo) {
        proximoContratiempo = -1;
        if (!palmera->tieneAtaquePendiente()) {
            palmera->preparar(static_cast<long long>(posCancionMs), true, static_cast<int>(mutantes.size()));
            lanzarAtaque(false);
        }
    }
}

void NivelJefe::lanzarAtaque(bool programarContratiempo) {
    std::vector<Proyectil*> nuevos;
    std::vector<BananaMutante*> invocados;
    int id = palmera->lanzar(static_cast<int>(mutantes.size()), Y_SUELO, nuevos, invocados);
    if (id < 0) return;

    for (Proyectil* p : nuevos) agregarProyectil(p);
    if (!nuevos.empty()) vivosPorAtaque[id] = static_cast<int>(nuevos.size());
    if (!invocados.empty()) {
        if (idAtaqueMutantes >= 0) palmera->notificarResultado(idAtaqueMutantes, false);
        mutantes.insert(mutantes.end(), invocados.begin(), invocados.end());
        idAtaqueMutantes = id;
    }
    eventos.push(EVENTO_ATAQUE_JEFE);

    if (programarContratiempo && intensidadActual == 3 &&
        std::uniform_real_distribution<double>(0, 1)(generador) < config.getProbContratiempo())
        proximoContratiempo = posCancionMs + msPorBeat / 2;
}

void NivelJefe::resolverColisiones() {
    int vidaAntes = rick->getVida();
    std::vector<Proyectil*> ondas;
    for (Proyectil* p : proyectiles) {
        if (!p->estaActiva()) continue;

        if (p->getTipo() == PROYECTIL_ONDA) {
            double distancia = std::fabs(rick->getCentroX() - p->getCentroX());
            if (p->getAlto() < 6) p->desactivar();
            else if (rick->estaEnSuelo() && rick->getY() + rick->getAlto() >= Y_SUELO - 1 && !rick->esInvulnerable() &&
                     std::fabs(distancia - p->getAncho() / 2) < 35)
                rick->recibirDanio(1);
            continue;
        }
        if (!p->esDelJugador()) {
            double piso = p->getY() + p->getAlto();
            if (p->getTipo() == PROYECTIL_RACIMO && !p->haExplotado() && piso >= Y_SUELO) {
                ondas.push_back(new Proyectil(p->getCentroX() - 30, Y_SUELO - 46, 0, 0, 60, 46, 1, false, PROYECTIL_ONDA,
                                              new FisicaOnda(p->getCentroX(), Y_SUELO, 500, 1.8, 46, 0.9), -1, 2.5));
                p->explotar(90);
            }
            if (p->getTipo() == PROYECTIL_HOJA && piso >= Y_SUELO) { p->desactivar(); continue; }
            if (p->getTipo() == PROYECTIL_BANANO && (p->getY() > ALTO_PANTALLA + 40 || p->getX() < -100 ||
                                                     p->getX() > ANCHO_PANTALLA + 100)) {
                p->desactivar();
                continue;
            }
            if (!rick->esInvulnerable() && p->colisionaCon(*rick)) {
                rick->recibirDanio(p->getDanio());
                p->desactivar();
                auto it = vivosPorAtaque.find(p->getIdAtaque());
                if (it != vivosPorAtaque.end()) {
                    palmera->notificarResultado(it->first, true);
                    vivosPorAtaque.erase(it);
                }
            }
            continue;
        }

        for (Proyectil* q : proyectiles) {
            if (!q->estaActiva() || q->getTipo() != PROYECTIL_RACIMO || q->haExplotado() || !p->colisionaCon(*q))
                continue;
            p->desactivar();
            q->desactivar();
            if (intensidadActual == 0) break;
            int danio = p->getDanio() * 15, faseAntes = palmera->getFase();
            palmera->recibirDanio(danio);
            rick->registrarImpacto();
            puntaje += danio;
            eventos.push(EVENTO_RACIMO_DESTRUIDO);
            if (palmera->getFase() != faseAntes) eventos.push(EVENTO_CAMBIO_FASE);
            break;
        }
        if (!p->estaActiva()) continue;

        for (BananaMutante* m : mutantes)
            if (m->estaActiva() && p->colisionaCon(*m)) {
                m->recibirDanio(1);
                p->desactivar();
                if (!m->estaVivo()) {
                    m->desactivar();
                    puntaje += 100;
                    eventos.push(EVENTO_ENEMIGO_DERROTADO);
                }
                break;
            }
    }

    for (Proyectil* o : ondas) agregarProyectil(o);

    for (BananaMutante* m : mutantes)
        if (m->estaActiva() && !rick->esInvulnerable() && m->colisionaCon(*rick)) {
            rick->recibirDanio(1);
            if (idAtaqueMutantes >= 0) {
                palmera->notificarResultado(idAtaqueMutantes, true);
                idAtaqueMutantes = -1;
            }
        }

    int i = 0;
    while (i < static_cast<int>(mutantes.size())) {
        if (mutantes[i]->estaActiva()) {
            i++;
            continue;
        }
        delete mutantes[i];
        mutantes[i] = mutantes.back();
        mutantes.pop_back();
    }

    if (idAtaqueMutantes >= 0 && mutantes.empty()) {
        palmera->notificarResultado(idAtaqueMutantes, false);
        idAtaqueMutantes = -1;
    }
    if (rick->getVida() < vidaAntes) eventos.push(EVENTO_DANIO_JUGADOR);
}

void NivelJefe::alEliminarProyectil(const Proyectil& p) {
    auto it = vivosPorAtaque.find(p.getIdAtaque());
    if (it == vivosPorAtaque.end()) return;
    if (--it->second == 0) {
        palmera->notificarResultado(it->first, false);
        vivosPorAtaque.erase(it);
    }
}

void NivelJefe::dispararHacia(double x, double y) {
    if (estado != NIVEL_EN_CURSO) return;
    Proyectil* p = rick->dispararHacia(x, y);
    if (!p) return;
    agregarProyectil(p);
    eventos.push(EVENTO_DISPARO);
}

void NivelJefe::procesarEntrada(int a, bool presionada) {
    switch (a) {
    case ACCION_IZQUIERDA: izquierda = presionada; break;
    case ACCION_DERECHA: derecha = presionada; break;
    case ACCION_SALTAR:
        if (presionada) rick->saltar();
        break;
    case ACCION_PORTAL:
        if (presionada) rick->teletransportar();
        break;
    default: break;
    }
}

const Rick& NivelJefe::getRick() const { return *rick; }
const PalmeraJefe& NivelJefe::getPalmera() const { return *palmera; }
PalmeraJefe& NivelJefe::getPalmera() { return *palmera; }
const std::vector<BananaMutante*>& NivelJefe::getMutantes() const { return mutantes; }
const ConfiguracionDificultad& NivelJefe::getConfiguracion() const { return config; }
double NivelJefe::getPosicionCancionMs() const { return posCancionMs; }
long long NivelJefe::getDuracionMs() const { return duracionMs; }
double NivelJefe::getMsPorBeat() const { return msPorBeat; }
const std::vector<std::array<double, 4>>& NivelJefe::getPlataformas() const { return plataformas; }
int NivelJefe::getIntensidadActual() const { return intensidadActual; }
