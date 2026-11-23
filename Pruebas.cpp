// Pruebas de la capa logica del juego (consola, sin Qt)
#include <cmath>
#include <fstream>
#include <iostream>
#include <sstream>
#include <stdexcept>
#include <string>

#include "configuraciondificultad.h"
#include "fisicaarrastre.h"
#include "fisicaonda.h"
#include "fisicaparabolica.h"
#include "fisicapendular.h"
#include "moduloaprendizaje.h"
#include "morty.h"
#include "niveljefe.h"
#include "nivelplantacion.h"
#include "palmerajefe.h"
#include "proyectil.h"
#include "rick.h"
#include "tipo.h"

#ifndef RUTA_MAPA
#define RUTA_MAPA "recursos/MapaRitmo.txt"
#endif

static int correctas = 0, total = 0, correctasTotal = 0, totalGeneral = 0;

static void verificar(bool condicion, const std::string& descripcion) {
    total++;
    if (condicion) correctas++;
    std::cout << "  " << (condicion ? "OK   " : "FALLA") << "   " << descripcion << "\n";
}

static void clase(const std::string& nombre) {
    correctas = 0;
    total = 0;
    std::cout << "\n---------- " << nombre << " ----------\n";
}

static void grupo(const std::string& nombre) { std::cout << nombre << "\n"; }

static void resumen(const std::string& nombre) {
    std::cout << "=> " << nombre << ": " << correctas << " de " << total << " correctas\n";
    correctasTotal += correctas;
    totalGeneral += total;
}

static bool cerca(double a, double b, double tol = 1e-6) { return std::fabs(a - b) < tol; }

// Entidad sencilla para probar las fisicas por separado
class Cuerpo : public Entidad {
public:
    Cuerpo(double vx, double vy, ModeloFisico* f) : Entidad(0, 0, 10, 10, f) { setVelocidad(vx, vy); }
    void actualizar(double dt) override { fisica->aplicar(*this, dt); }
};

static void cargarMapa(NivelJefe& n) {
    std::ifstream archivo(RUTA_MAPA);
    if (!archivo) throw std::runtime_error(std::string("no se encontro ") + RUTA_MAPA);
    n.cargarMapaRitmo(archivo);
}

// ---------------------------------------------------------------------------
static void pruebaParabolica() {
    clase("FisicaParabolica");
    grupo("Validacion de parametros");
    bool lanza = false;
    try { FisicaParabolica f(0); } catch (const std::invalid_argument&) { lanza = true; }
    verificar(lanza, "gravedad 0 lanza excepcion");

    grupo("Un paso de fisica (Euler semi-implicito)");
    Cuerpo c(100, 0, new FisicaParabolica(1400));
    c.actualizar(0.1);
    verificar(cerca(c.getVy(), 140), "la velocidad vertical aumenta g * dt = 140");
    verificar(cerca(c.getX(), 10), "x avanza con velocidad constante (100 * 0.1 = 10)");

    grupo("Salto de Morty (v0y = -750)");
    Cuerpo salto(0, -750, new FisicaParabolica(1400));
    double alturaMax = 0;
    for (int i = 0; i < 1000; i++) {
        salto.actualizar(0.001);
        if (-salto.getY() > alturaMax) alturaMax = -salto.getY();
    }
    verificar(cerca(alturaMax, 750.0 * 750 / (2 * 1400), 1), "altura maxima = v0^2 / 2g = 200.9 px");
    verificar(salto.getVy() > 0, "despues de 1 s ya esta cayendo");

    grupo("Racimo apuntado");
    double vx, vy;
    FisicaParabolica::velocidadApuntado(300, 400, 1.2, 1400, vx, vy);
    Cuerpo racimo(vx, vy, new FisicaParabolica(1400));
    for (int i = 0; i < 1200; i++) racimo.actualizar(0.001);
    verificar(cerca(racimo.getX(), 300, 3) && cerca(racimo.getY(), 400, 3), "cae en el objetivo (300, 400) en T = 1.2 s");
    lanza = false;
    try { FisicaParabolica::velocidadApuntado(1, 1, 0, 1400, vx, vy); } catch (const std::invalid_argument&) { lanza = true; }
    verificar(lanza, "tiempo de vuelo 0 lanza excepcion");
    resumen("FisicaParabolica");
}

static void pruebaPendular() {
    clase("FisicaPendular");
    grupo("Validacion de parametros");
    bool lanza = false;
    try { FisicaPendular f(0, 0, -5); } catch (const std::invalid_argument&) { lanza = true; }
    verificar(lanza, "longitud negativa lanza excepcion");

    grupo("Posicion");
    FisicaPendular quieto(100, 50, 400);
    verificar(cerca(quieto.getPuntoX(), 100) && cerca(quieto.getPuntoY(), 450), "en reposo cuelga debajo del ancla (100, 450)");

    grupo("Oscilacion de la liana (L = 400)");
    FisicaPendular p(0, 0, 400, 0.1, 0.0, 1400);
    double t = 0, antes = p.getTheta(), cruces[3];
    int n = 0;
    while (n < 3 && t < 10) {
        p.avanzar(0.0001);
        t += 0.0001;
        if (antes > 0 && p.getTheta() <= 0) cruces[n++] = t;
        antes = p.getTheta();
    }
    double periodo = (cruces[2] - cruces[0]) / 2, esperado = 2 * PI * std::sqrt(400.0 / 1400);
    verificar(std::fabs(periodo - esperado) / esperado < 0.01, "periodo = 2*pi*sqrt(L/g) = 3.36 s");

    FisicaPendular amortiguado(0, 0, 400, 0.6, 0.6);
    for (int i = 0; i < 6000; i++) amortiguado.avanzar(1.0 / 600);
    verificar(std::fabs(amortiguado.getTheta()) < 0.6, "con amortiguamiento la amplitud disminuye");

    FisicaPendular impulso(0, 0, 400);
    impulso.impulsar(1.0);
    verificar(cerca(impulso.getOmega(), 1.0), "impulsar suma velocidad angular");

    grupo("Soltarse de la liana");
    double vx, vy;
    impulso.velocidadTangencial(vx, vy);
    verificar(cerca(vx, 400) && cerca(vy, 0), "en el punto mas bajo sale horizontal (L * omega = 400)");

    grupo("Hoja con ancla que baja");
    FisicaPendular hoja(0, 0, 70, 0, 0.3, 1400, 240);
    for (int i = 0; i < 60; i++) hoja.avanzar(1.0 / 60);
    verificar(cerca(hoja.getYAncla(), 240, 0.01), "el ancla baja 240 px en 1 s");
    resumen("FisicaPendular");
}

static void pruebaArrastre() {
    clase("FisicaArrastre");
    grupo("Validacion de parametros");
    bool lanza = false;
    try { FisicaArrastre f(0); } catch (const std::invalid_argument&) { lanza = true; }
    verificar(lanza, "coeficiente 0 lanza excepcion");

    grupo("Banano lanzado (v0 = 720, c = 0.8)");
    FisicaArrastre* f = new FisicaArrastre(0.8);
    Cuerpo b(720, 0, f);
    for (int i = 0; i < 180; i++) b.actualizar(1.0 / 60);
    verificar(cerca(b.getX(), 900 * (1 - std::exp(-2.4))), "distancia a los 3 s = solucion exacta (818.35)");
    verificar(cerca(b.getVx(), 720 * std::exp(-2.4)), "la velocidad baja como v0 * e^(-ct)");
    verificar(cerca(f->alcanceMaximo(720), 900), "alcance maximo = v0 / c = 900");

    grupo("Independencia del paso de tiempo");
    Cuerpo b1(720, 0, new FisicaArrastre(0.8)), b2(720, 0, new FisicaArrastre(0.8));
    b1.actualizar(1.0);
    for (int i = 0; i < 100; i++) b2.actualizar(0.01);
    verificar(cerca(b1.getX(), b2.getX()), "1 paso de 1 s = 100 pasos de 0.01 s");
    resumen("FisicaArrastre");
}

static void pruebaOnda() {
    clase("FisicaOnda");
    grupo("Validacion de parametros");
    bool lanza = false;
    try { FisicaOnda f(0, 0, -1, 1, 1, 1); } catch (const std::invalid_argument&) { lanza = true; }
    verificar(lanza, "velocidad negativa lanza excepcion");

    grupo("Expansion de la onda (v0 = 500, k = 1.8)");
    FisicaOnda* f = new FisicaOnda(640, 700, 500, 1.8, 46, 0.9);
    Cuerpo c(0, 0, f);
    verificar(cerca(f->getRadio(), 30) && cerca(f->getAmplitud(), 46), "empieza con radio 30 y altura 46");
    c.actualizar(0.5);
    double radio1 = f->getRadio(), altura1 = f->getAmplitud(), vel1 = f->getVelocidadFrente();
    c.actualizar(0.5);
    verificar(f->getRadio() > radio1, "el radio crece");
    verificar(f->getAmplitud() < altura1, "la altura disminuye");
    verificar(f->getVelocidadFrente() < vel1, "el frente va frenando");
    verificar(cerca(c.getAncho(), 2 * f->getRadio()) && cerca(c.getAlto(), f->getAmplitud()), "la entidad toma el tamano de la onda");
    for (int i = 0; i < 50; i++) c.actualizar(0.1);
    verificar(cerca(f->getRadio(), f->radioMaximo(), 0.5), "el radio tiende a r0 + v0/k = 307.8");
    resumen("FisicaOnda");
}

// ---------------------------------------------------------------------------
static void pruebaMorty() {
    clase("Morty");
    grupo("Estado inicial");
    Morty m(0, 0);
    verificar(m.getVida() == 5, "empieza con 5 vidas");
    verificar(m.getValor() == 0 && !m.estaEnValentia(), "empieza sin valor y sin valentia");

    grupo("Nerviosismo (dispersion del disparo)");
    verificar(m.calcularDispersion() == 0, "sin enemigos cerca no tiembla");
    m.setEnemigosCerca(3);
    verificar(m.calcularDispersion() == 12, "3 enemigos cerca: 12 grados");
    m.setEnemigosCerca(10);
    verificar(m.calcularDispersion() == 25, "la dispersion no pasa de 25 grados");

    grupo("Valor y valentia");
    m.registrarLiberado();
    verificar(m.getValor() == 34 && m.getLiberados() == 1, "liberar un trabajador suma 34 de valor");
    m.registrarLiberado();
    m.registrarLiberado();
    verificar(m.estaEnValentia(), "3 liberados: entra en valentia");
    verificar(m.calcularDispersion() == 0, "en valentia no tiembla");

    grupo("Cansancio");
    for (int i = 0; i < 485; i++) m.actualizar(1.0 / 60);
    verificar(!m.estaEnValentia() && m.estaCansado(), "despues de 8 s queda cansado");
    for (int i = 0; i < 245; i++) m.actualizar(1.0 / 60);
    verificar(!m.estaCansado(), "el cansancio dura 4 s");

    grupo("Disparo");
    Morty d(0, 0);
    Proyectil* p1 = d.disparar();
    Proyectil* p2 = d.disparar();
    verificar(p1 != nullptr && p1->esDelJugador(), "dispara un proyectil del jugador");
    verificar(p2 == nullptr, "no puede disparar otra vez durante la recarga");
    delete p1;
    delete p2;
    d.setLiberando(true);
    verificar(d.disparar() == nullptr, "mientras libera una jaula no puede disparar");

    grupo("Vida");
    Morty v(0, 0);
    v.recibirDanio(1);
    v.recibirDanio(1);
    verificar(v.getVida() == 4, "despues de un golpe queda invulnerable un momento");
    resumen("Morty");
}

static void pruebaRick() {
    clase("Rick");
    grupo("Estado inicial");
    Rick r(500, 580, 60, 1160);
    verificar(r.getVida() == 6, "empieza con 6 vidas");

    grupo("Disparo con el mouse");
    Proyectil* p = r.dispararHacia(530, 100);
    verificar(p != nullptr && cerca(p->getVx(), 0, 1) && p->getVy() < 0, "apuntando arriba el rayo sube");
    verificar(p && p->getDanio() == 10, "el rayo hace 10 de dano");
    verificar(r.dispararHacia(530, 100) == nullptr, "no puede disparar otra vez durante la recarga");
    delete p;

    grupo("Combo");
    for (int i = 0; i < 3; i++) r.registrarImpacto();
    verificar(r.getCombo() == 3 && cerca(r.getMultiplicador(), 1.3), "3 impactos seguidos: multiplicador 1.3");
    for (int i = 0; i < 20; i++) r.registrarImpacto();
    verificar(cerca(r.getMultiplicador(), 2.0), "el multiplicador no pasa de 2");

    grupo("Recibir dano");
    r.recibirDanio(1);
    verificar(r.getVida() == 5, "pierde una vida");
    verificar(r.getCombo() == 0 && cerca(r.getMultiplicador(), 1.0), "se rompe el combo");
    verificar(r.estaAturdido(), "queda aturdido");

    grupo("Portal");
    Rick q(500, 580, 60, 1160);
    verificar(q.teletransportar() && cerca(q.getX(), 720), "el portal avanza 220 px");
    verificar(q.esInvulnerable(), "al usar el portal es invulnerable un momento");
    verificar(!q.teletransportar(), "no se puede usar de nuevo durante la recarga (3 s)");
    Rick borde(1100, 580, 60, 1160);
    borde.teletransportar();
    verificar(cerca(borde.getX(), 1160), "el portal no lo saca de la pantalla");
    resumen("Rick");
}

// ---------------------------------------------------------------------------
static void pruebaDificultad() {
    clase("ConfiguracionDificultad");
    grupo("Validacion de parametros");
    bool lanza = false;
    try { ConfiguracionDificultad c("Mala", 2.0, 0.1, 1, 500, 0.1, 1.0); } catch (const std::invalid_argument&) { lanza = true; }
    verificar(lanza, "tasa de aprendizaje 2 lanza excepcion");

    grupo("Las dificultades cambian al agente");
    ConfiguracionDificultad f = ConfiguracionDificultad::facil(), n = ConfiguracionDificultad::normal(),
        d = ConfiguracionDificultad::dificil();
    verificar(f.getTasaAprendizaje() < n.getTasaAprendizaje() && n.getTasaAprendizaje() < d.getTasaAprendizaje(),
              "aprende mas rapido: 0.05 < 0.15 < 0.30");
    verificar(f.getEpsilon() > n.getEpsilon() && n.getEpsilon() > d.getEpsilon(), "explora menos: 0.40 > 0.20 > 0.08");
    verificar(f.getAvisoMs() > n.getAvisoMs() && n.getAvisoMs() > d.getAvisoMs(), "avisa con menos tiempo: 800 > 600 > 450 ms");
    verificar(f.getPrediccion() == 0 && d.getPrediccion() == 2, "en facil no predice, en dificil predice 2 pasos");
    verificar(f.getProbContratiempo() == 0, "en facil no ataca a contratiempo");
    resumen("ConfiguracionDificultad");
}

static void pruebaAprendizaje() {
    clase("ModuloAprendizaje");
    grupo("Estado inicial");
    ModuloAprendizaje a;
    verificar(cerca(a.getPeso(0, ATAQUE_BANANOS), 0.5) && cerca(a.getPeso(11, ATAQUE_MUTANTES), 0.5), "todos los pesos empiezan en 0.5");

    grupo("Regla w <- w + alfa(r - w), alfa = 0.15");
    a.registrarPendiente(1, 5, ATAQUE_BANANOS);
    a.resolver(1, false, 0.15);
    verificar(cerca(a.getPeso(5, ATAQUE_BANANOS), 0.425), "Rick esquiva: 0.5 -> 0.425");
    a.registrarPendiente(2, 5, ATAQUE_RACIMOS);
    a.resolver(2, true, 0.15);
    verificar(cerca(a.getPeso(5, ATAQUE_RACIMOS), 0.575), "el ataque acierta: 0.5 -> 0.575");
    verificar(cerca(a.getPeso(4, ATAQUE_BANANOS), 0.5), "otro contexto no cambia");
    a.resolver(1, true, 0.15);
    verificar(cerca(a.getPeso(5, ATAQUE_BANANOS), 0.425), "un ataque se resuelve una sola vez");

    grupo("Convergencia");
    ModuloAprendizaje b;
    for (int i = 0; i < 60; i++) {
        b.registrarPendiente(i, 0, ATAQUE_HOJAS);
        b.resolver(i, true, 0.15);
    }
    verificar(b.getPeso(0, ATAQUE_HOJAS) > 0.99, "un ataque que siempre acierta tiende a 1");
    verificar(b.getUsos(ATAQUE_HOJAS) == 60 && cerca(b.getTasaAcierto(ATAQUE_HOJAS), 1), "cuenta 60 usos con 100 % de acierto");

    grupo("Guardar y cargar la memoria");
    std::stringstream memoria;
    a.guardar(memoria);
    ModuloAprendizaje c;
    c.cargar(memoria);
    verificar(cerca(c.getPeso(5, ATAQUE_BANANOS), 0.425, 1e-4), "la memoria cargada es igual a la guardada");
    std::stringstream danada("0.5 0.5 abc");
    bool lanza = false;
    try { c.cargar(danada); } catch (const std::runtime_error&) { lanza = true; }
    verificar(lanza, "una memoria danada lanza excepcion");
    verificar(cerca(c.getPeso(5, ATAQUE_BANANOS), 0.425, 1e-4), "y no altera la memoria que habia");
    resumen("ModuloAprendizaje");
}

// ---------------------------------------------------------------------------
static void pruebaNivelJefe() {
    clase("NivelJefe (nivel 2)");
    ConfiguracionDificultad normal = ConfiguracionDificultad::normal();

    grupo("Mapa de ritmo");
    NivelJefe n(normal, 7);
    cargarMapa(n);
    verificar(cerca(n.getMsPorBeat(), 480), "125 BPM: 480 ms por beat");
    verificar(n.getDuracionMs() == 170000, "la cancion dura 170 s");
    bool lanza = false;
    std::stringstream malo("125 170000\n500 0\n400 1\n");
    try { NivelJefe m(normal, 7); m.cargarMapaRitmo(malo); } catch (const std::runtime_error&) { lanza = true; }
    verificar(lanza, "beats fuera de orden lanzan excepcion");
    lanza = false;
    try { NivelJefe m(normal, 7); m.actualizar(0.016); } catch (const std::runtime_error&) { lanza = true; }
    verificar(lanza, "jugar sin mapa cargado lanza excepcion");

    grupo("Estado inicial");
    verificar(n.getEstado() == NIVEL_EN_CURSO && n.getPalmera().getVida() == 7500, "la palmera empieza con 7500 de vida");
    verificar(n.getPlataformas().size() == 2, "hay 2 plataformas");

    grupo("Introduccion de la cancion");
    int ataques = 0;
    for (int i = 0; i < 540; i++) {
        n.actualizar(1.0 / 60);
        int e;
        while (n.extraerEvento(e))
            if (e == EVENTO_ATAQUE_JEFE) ataques++;
    }
    verificar(ataques == 0, "en los primeros 9 s (intensidad 0) la palmera no ataca");

    grupo("Dano a la palmera");
    NivelJefe tronco(normal, 7);
    cargarMapa(tronco);
    const PalmeraJefe& p = tronco.getPalmera();
    for (int i = 0; i < 180; i++) {
        tronco.dispararHacia(p.getCentroX(), p.getCentroY());
        tronco.actualizar(1.0 / 60);
    }
    verificar(p.getVida() == 7500, "dispararle al tronco no le hace dano");

    NivelJefe racimos(normal, 7);
    cargarMapa(racimos);
    racimos.sincronizar(30000);
    int destruidos = 0;
    for (int i = 0; i < 600 && destruidos == 0; i++) {
        const Proyectil* obj = nullptr;
        for (Proyectil* q : racimos.getProyectiles())
            if (q->estaActiva() && q->getTipo() == PROYECTIL_RACIMO && !q->haExplotado() && q->getVy() < 300) obj = q;
        if (obj) racimos.dispararHacia(obj->getCentroX() + obj->getVx() * 0.25, obj->getCentroY() + obj->getVy() * 0.25);
        racimos.actualizar(1.0 / 60);
        int e;
        while (racimos.extraerEvento(e))
            if (e == EVENTO_RACIMO_DESTRUIDO) destruidos++;
    }
    verificar(destruidos == 1 && racimos.getPalmera().getVida() == 7350, "destruir un racimo le quita 150 (10 x 15)");

    grupo("Fases");
    NivelJefe fases(normal, 7);
    fases.getPalmera().recibirDanio(3700);
    verificar(fases.getPalmera().getFase() == 1, "con 3800 de vida sigue en fase 1");
    fases.getPalmera().recibirDanio(100);
    verificar(fases.getPalmera().getFase() == 2, "bajo el 50 % pasa a fase 2");

    grupo("Fin del nivel");
    NivelJefe fin(normal, 7);
    cargarMapa(fin);
    fin.sincronizar(170000);
    fin.actualizar(1.0 / 60);
    verificar(fin.getEstado() == NIVEL_PERDIDO, "si termina la cancion, Rick pierde");
    NivelJefe gana(normal, 7);
    cargarMapa(gana);
    gana.getPalmera().recibirDanio(7500);
    gana.actualizar(1.0 / 60);
    verificar(gana.getEstado() == NIVEL_GANADO, "si la palmera llega a 0, Rick gana");
    verificar(gana.getPuntaje() == 6 * 500, "al ganar suma 500 por cada vida (6 x 500)");
    resumen("NivelJefe");
}

static void pruebaNivelPlantacion() {
    clase("NivelPlantacion (nivel 1)");
    grupo("Configuracion del mapa");
    NivelPlantacion n;
    verificar(n.getXJaulas().size() == 9, "hay 9 jaulas");
    verificar(n.getLianas().size() == 12, "hay 12 lianas");
    verificar(n.getBananas().size() == 25, "hay 25 enemigos (11 mutantes y 14 soldados)");
    verificar(NivelPlantacion::MINIMO_LIBERADOS == 6, "hay que liberar minimo 6 trabajadores");

    grupo("Movimiento");
    for (int i = 0; i < 60; i++) n.actualizar(1.0 / 60);
    verificar(cerca(n.getMorty().getY() + n.getMorty().getAlto(), 640) && n.getMorty().estaEnSuelo(), "Morty se para sobre el suelo");
    double x0 = n.getMorty().getX();
    n.procesarEntrada(ACCION_DERECHA, true);
    for (int i = 0; i < 30; i++) n.actualizar(1.0 / 60);
    n.procesarEntrada(ACCION_DERECHA, false);
    verificar(cerca(n.getMorty().getX() - x0, 130, 5), "camina 260 px/s (130 px en 0.5 s)");

    grupo("Liberar un trabajador");
    NivelPlantacion j;
    j.procesarEntrada(ACCION_DERECHA, true);
    int i = 0;
    while (j.getMorty().getCentroX() < 725 && i++ < 600) j.actualizar(1.0 / 60);
    j.procesarEntrada(ACCION_DERECHA, false);
    j.procesarEntrada(ACCION_INTERACTUAR, true);
    for (int k = 0; k < 60; k++) j.actualizar(1.0 / 60);
    verificar(!j.jaulaLiberada(0), "con 1 s presionando E la jaula sigue cerrada");
    for (int k = 0; k < 40; k++) j.actualizar(1.0 / 60);
    verificar(j.jaulaLiberada(0) && j.getMorty().getLiberados() == 1, "a los 1.5 s la jaula se abre");
    verificar(j.getPuntaje() >= 500, "liberar un trabajador da 500 puntos");
    resumen("NivelPlantacion");
}

int main() {
    std::cout << "========== PRUEBAS DE LA LOGICA DEL JUEGO ==========\n";
    try {
        pruebaParabolica();
        pruebaPendular();
        pruebaArrastre();
        pruebaOnda();
        pruebaMorty();
        pruebaRick();
        pruebaDificultad();
        pruebaAprendizaje();
        pruebaNivelJefe();
        pruebaNivelPlantacion();
    } catch (const std::exception& e) {
        std::cout << "\nERROR: " << e.what() << "\n";
        return 1;
    }
    std::cout << "\n========== TOTAL: " << correctasTotal << " de " << totalGeneral << " pruebas correctas ==========\n";
    return correctasTotal == totalGeneral ? 0 : 1;
}
