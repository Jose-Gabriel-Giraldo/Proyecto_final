#include "bananamutante.h"
#include "configuraciondificultad.h"
#include "fisicaarrastre.h"
#include "fisicaparabolica.h"
#include "fisicapendular.h"
#include "morty.h"
#include "proyectil.h"
#include "rick.h"
#include <cmath>
#include <iomanip>
#include <iostream>
#include <vector>

const double SUELO = 640;
const double DT = 1.0 / 60;

static void limpiar(std::vector<Proyectil*>& proyectiles) {
    for (size_t i = 0; i < proyectiles.size();) {
        if (proyectiles[i]->estaActiva()) { i++; continue; }
        delete proyectiles[i];
        proyectiles[i] = proyectiles.back();
        proyectiles.pop_back();
    }
}

static void escenaMorty() {
    std::cout << "\n===== ESCENA 1: Morty en la plantacion =====\n";
    Morty morty(100, SUELO - 90);
    std::vector<BananaMutante*> bananas = {new BananaMutante(700, SUELO - 110, 500, 1000),
                                            new BananaMutante(1300, SUELO - 110, 1300, 1370, true, 0, 500, 3)};
    std::vector<Proyectil*> proyectiles;
    FisicaPendular liana(1700, 100, 400, 0.0, 0.6);
    double xJaula = 1500, progreso = 0;
    int derrotados = 0;
    bool usoLiana = false;

    for (int frame = 0; frame < 60 * 14; frame++) {
        double t = frame * DT;
        liana.avanzar(DT);

        int cerca = 0;
        for (BananaMutante* b : bananas)
            if (b->estaActiva() && std::fabs(b->getCentroX() - morty.getCentroX()) < 300) cerca++;
        morty.setEnemigosCerca(cerca);

        bool liberando = progreso < 1.5 && std::fabs(xJaula - morty.getCentroX()) < 70 && morty.estaEnSuelo();
        morty.setLiberando(liberando);
        if (liberando) {
            progreso += DT;
            if (progreso >= 1.5) {
                morty.registrarLiberado();
                std::cout << std::fixed << std::setprecision(2) << "[" << t << " s] Morty libera al trabajador\n";
            }
        }

        morty.mover(liberando ? 0 : 1);
        Proyectil* disparo = morty.disparar();
        if (disparo) proyectiles.push_back(disparo);

        if (!usoLiana && !morty.getLiana() && std::fabs(liana.getPuntoX() - morty.getCentroX()) < 80 &&
            std::fabs(liana.getPuntoY() - morty.getCentroY()) < 80) {
            morty.agarrarLiana(&liana);
            usoLiana = true;
            std::cout << "[" << t << " s] Morty se agarra a la liana\n";
        }
        bool colgado = morty.getLiana() != nullptr;

        if (frame % 50 == 25) morty.saltar();
        if (colgado && !morty.getLiana())
            std::cout << "[" << t << " s] Morty salta de la liana con vx = " << morty.getVx() << "\n";

        double piesAntes = morty.getY() + morty.getAlto();
        morty.actualizar(DT);
        if (!morty.getLiana() && morty.getY() + morty.getAlto() >= SUELO && piesAntes <= SUELO + 1) morty.aterrizar(SUELO);

        for (BananaMutante* b : bananas) {
            if (!b->estaActiva()) continue;
            b->comportarse(morty, DT);
            b->actualizar(DT);
            Proyectil* bala = b->disparar();
            if (bala) proyectiles.push_back(bala);
            if (b->colisionaCon(morty)) morty.recibirDanio(1);
        }

        for (Proyectil* p : proyectiles) {
            p->actualizar(DT);
            if (!p->estaActiva()) continue;
            if (!p->esDelJugador() && p->colisionaCon(morty)) {
                morty.recibirDanio(p->getDanio());
                p->desactivar();
                continue;
            }
            for (BananaMutante* b : bananas)
                if (p->esDelJugador() && p->colisionaCon(*b)) {
                    b->recibirDanio(p->getDanio());
                    p->desactivar();
                    if (!b->estaVivo()) {
                        b->desactivar();
                        derrotados++;
                        std::cout << "[" << t << " s] Banana " << (b->esArmado() ? "armada" : "sin arma") << " derrotada\n";
                    }
                    break;
                }
        }
        limpiar(proyectiles);

        if (frame % 60 == 0)
            std::cout << "[" << t << " s] Morty x = " << std::setw(7) << morty.getX() << "  vida " << morty.getVida()
                      << "  valor " << morty.getValor() << "  dispersion " << morty.calcularDispersion()
                      << "  proyectiles " << proyectiles.size() << "\n";
    }

    std::cout << "Resultado: Morty llego a x = " << morty.getX() << " con " << morty.getVida() << " vidas, "
              << morty.getLiberados() << " liberado(s), " << derrotados << " banana(s) derrotada(s)\n";
    for (BananaMutante* b : bananas) delete b;
    for (Proyectil* p : proyectiles) delete p;
}

static void escenaRick(const ConfiguracionDificultad& config) {
    std::cout << "\n===== ESCENA 2: Rick esquiva los ataques (" << config.getNombre() << ") =====\n";
    Rick rick(150, SUELO - 120, 40, 700);
    rick.aterrizar(SUELO);
    std::vector<Proyectil*> proyectiles;
    int golpes = 0, impactosRick = 0;
    double xBoca = 1030, yBoca = 330;

    for (int frame = 0; frame < 60 * 10; frame++) {
        double t = frame * DT;

        if (frame % 120 == 0) {
            double vx, vy, T = config.getTiempoVueloRacimo();
            FisicaParabolica::velocidadApuntado(rick.getCentroX() - 25 - xBoca, SUELO - 60 - yBoca, T, 1400, vx, vy);
            proyectiles.push_back(new Proyectil(xBoca, yBoca, vx, vy, 50, 60, 1, false, PROYECTIL_RACIMO,
                                                new FisicaParabolica(1400), 1, T + 1));
            std::cout << "[" << t << " s] La palmera lanza un racimo apuntado a x = " << rick.getCentroX() << "\n";
        }
        if (frame % 120 == 60) {
            double c = 900.0 / (xBoca - rick.getCentroX());
            if (c < 1.2) c = 1.2;
            if (c > 2.0) c = 2.0;
            proyectiles.push_back(new Proyectil(xBoca, SUELO - 70, -900, 0, 40, 20, 1, false, PROYECTIL_BANANO,
                                                new FisicaArrastre(c), 2, 3));
            std::cout << "[" << t << " s] La palmera dispara un banano con alcance "
                      << FisicaArrastre(c).alcanceMaximo(900) << " px\n";
        }
        if (frame == 300)
            for (int i = 0; i < 4; i++)
                proyectiles.push_back(new Proyectil(0, 0, 0, 0, 26, 18, 1, false, PROYECTIL_HOJA,
                                                    new FisicaPendular(100 + i * 160, -100, 70, 0.5, 0.3, 1400, 240), 3, 8));

        rick.mover(frame % 120 < 60 ? 1 : -1);
        if (frame % 120 == 50) rick.saltar();
        if (frame % 240 == 30 || frame % 240 == 210) rick.teletransportar();
        Proyectil* rayo = rick.disparar();
        if (rayo) proyectiles.push_back(rayo);

        rick.actualizar(DT);
        if (rick.getY() + rick.getAlto() >= SUELO) rick.aterrizar(SUELO);

        for (Proyectil* p : proyectiles) {
            p->actualizar(DT);
            if (!p->estaActiva()) continue;
            if (p->esDelJugador()) {
                if (p->getX() > 1000) { rick.registrarImpacto(); impactosRick++; p->desactivar(); }
                continue;
            }
            if (p->getTipo() == PROYECTIL_RACIMO && !p->haExplotado() && p->getY() + p->getAlto() >= SUELO) p->explotar(60);
            if (p->getTipo() == PROYECTIL_HOJA && p->getY() + p->getAlto() >= SUELO) { p->desactivar(); continue; }
            if (!rick.esInvulnerable() && p->colisionaCon(rick)) {
                rick.recibirDanio(1);
                p->desactivar();
                golpes++;
                std::cout << "[" << t << " s] Rick recibe un golpe, vida " << rick.getVida() << "\n";
            }
        }
        limpiar(proyectiles);
    }

    std::cout << "Resultado: Rick termino con " << rick.getVida() << " vidas tras " << golpes << " golpe(s); acerto "
              << impactosRick << " rayos a la palmera, combo final x" << rick.getMultiplicador() << "\n";
    for (Proyectil* p : proyectiles) delete p;
}

int main() {
    std::cout << std::fixed << std::setprecision(2);
    escenaMorty();
    escenaRick(ConfiguracionDificultad::facil());
    escenaRick(ConfiguracionDificultad::dificil());
    std::cout << "\nPrograma terminado sin errores\n";
    return 0;
}
