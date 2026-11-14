#include "bananamutante.h"
#include "configuraciondificultad.h"
#include "moduloaprendizaje.h"
#include "modulopercepcion.h"
#include "niveljefe.h"
#include "palmerajefe.h"
#include "proyectil.h"
#include "rick.h"
#include <cmath>
#include <fstream>
#include <iostream>
#include <sstream>
#include <stdexcept>

static const char* nombreAtaque(int a) {
    if (a == ATAQUE_BANANOS) return "bananos";
    if (a == ATAQUE_RACIMOS) return "racimos";
    if (a == ATAQUE_HOJAS) return "hojas";
    return "mutantes";
}

static void pelea(const ConfiguracionDificultad& config) {
    std::cout << "\n===== NIVEL 2: Rick contra la Palmera Madre (" << config.getNombre() << ") =====" << std::endl;
    std::ifstream archivo("../../MapaRitmo.txt");
    if (!archivo) throw std::runtime_error("No se encontro ../../MapaRitmo.txt");
    NivelJefe nivel(config, 42);
    nivel.cargarMapaRitmo(archivo);
    std::cout << "Mapa: " << 60000.0 / nivel.getMsPorBeat() << " BPM, cancion de " << nivel.getDuracionMs() / 1000.0 << " s" << std::endl;

    const Rick& rick = nivel.getRick();
    const PalmeraJefe& palmera = nivel.getPalmera();
    int evento, intensidadAnterior = -1, ataques = 0, golpesRick = 0;
    nivel.procesarEntrada(ACCION_DISPARAR, true);

    for (int frame = 0; nivel.getEstado() == NIVEL_EN_CURSO; frame++) {
        int dir = 0;
        bool saltar = false;
        for (Proyectil* p : nivel.getProyectiles()) {
            if (p->esDelJugador()) continue;
            double dx = p->getCentroX() - rick.getCentroX();
            if (p->getTipo() == PROYECTIL_BANANO && dx > 0 && dx < 140 && p->getY() > 540) saltar = true;
            if (p->getTipo() != PROYECTIL_BANANO && std::fabs(dx) < 90 && p->getY() > 250) dir = dx > 0 ? -1 : 1;
        }
        for (BananaMutante* m : nivel.getMutantes())
            if (m->estaActiva() && m->getCentroX() > rick.getCentroX() && m->getCentroX() - rick.getCentroX() < 150) saltar = true;
        nivel.procesarEntrada(ACCION_IZQUIERDA, dir < 0);
        nivel.procesarEntrada(ACCION_DERECHA, dir > 0);
        if (saltar) nivel.procesarEntrada(ACCION_SALTAR, true);
        if (frame % 200 == 0) nivel.procesarEntrada(ACCION_PORTAL, true);

        nivel.actualizar(1.0 / 60);
        double s = nivel.getPosicionCancionMs() / 1000.0;
        while (nivel.extraerEvento(evento)) {
            if (evento == EVENTO_ATAQUE_JEFE) ataques++;
            if (evento == EVENTO_DANIO_JUGADOR) golpesRick++;
            if (evento == EVENTO_CAMBIO_FASE) std::cout << "  [" << s << " s] LA PALMERA PASA A FASE 2" << std::endl;
        }
        if (nivel.getIntensidadActual() != intensidadAnterior) {
            intensidadAnterior = nivel.getIntensidadActual();
            std::cout << "  [" << s << " s] la musica cambia a intensidad " << intensidadAnterior << "  (ataques hasta ahora "
                      << ataques << ", vida Rick " << rick.getVida() << ", vida palmera " << palmera.getVida() << ")" << std::endl;
        }
    }

    const ModuloAprendizaje& memoria = palmera.getAprendizaje();
    std::cout << "Resultado: " << (nivel.getEstado() == NIVEL_GANADO ? "GANADO" : "PERDIDO") << " a los "
              << nivel.getPosicionCancionMs() / 1000.0 << " s de la cancion, puntaje " << nivel.getPuntaje() << std::endl;
    std::cout << "  " << ataques << " ataques, Rick recibio " << golpesRick << " golpes, vida final de la palmera "
              << palmera.getVida() << std::endl;
    std::cout << "  Uso y acierto por ataque:";
    for (int a = 0; a < NUM_ATAQUES; a++)
        std::cout << "  " << nombreAtaque(a) << " " << memoria.getUsos(a) << " (" << memoria.getTasaAcierto(a) * 100 << " %)";
    std::cout << std::endl;
}

int main() {
    pelea(ConfiguracionDificultad::facil());
    pelea(ConfiguracionDificultad::dificil());

    std::cout << "\n===== MAPAS DE RITMO INVALIDOS =====" << std::endl;
    ConfiguracionDificultad normal = ConfiguracionDificultad::normal();
    NivelJefe nivel(normal, 1);
    const char* malos[] = {"125 170000\n1000 1\n900 1\n", "125 170000\n1000 7\n", "125 170000\n1000\n", "0 170000\n20 0\n"};
    for (const char* texto : malos) {
        std::istringstream mapa(texto);
        try {
            nivel.cargarMapaRitmo(mapa);
        } catch (const std::runtime_error& e) {
            std::cout << "  runtime_error: " << e.what() << std::endl;
        }
    }
    try {
        nivel.actualizar(1.0 / 60);
    } catch (const std::runtime_error& e) {
        std::cout << "  runtime_error: " << e.what() << std::endl;
    }
    std::cout << "\nPrograma terminado sin errores" << std::endl;
    return 0;
}
