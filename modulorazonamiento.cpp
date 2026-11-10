#include "modulorazonamiento.h"
#include "configuraciondificultad.h"
#include "moduloaprendizaje.h"
#include "modulopercepcion.h"
#include <array>

ModuloRazonamiento::ModuloRazonamiento(const ModuloPercepcion& percepcion, const ModuloAprendizaje& aprendizaje,
                                       unsigned semilla)
    : percepcion(&percepcion), aprendizaje(&aprendizaje), ataque(ATAQUE_BANANOS), objetivoX(0), contexto(0),
    generador(semilla) {}

void ModuloRazonamiento::decidir(int fase, bool contratiempo, int mutantesVivos, const ConfiguracionDificultad& config) {
    contexto = percepcion->getContexto();
    std::array<bool, NUM_ATAQUES> permitido;
    permitido[ATAQUE_BANANOS] = true;
    permitido[ATAQUE_RACIMOS] = !contratiempo;
    permitido[ATAQUE_HOJAS] = true;
    permitido[ATAQUE_MUTANTES] = fase == 2 && !contratiempo && mutantesVivos < 3;
    std::array<double, NUM_ATAQUES> peso{};
    double total = 0;
    for (int a = 0; a < NUM_ATAQUES; a++)
        if (permitido[a]) {
            peso[a] = aprendizaje->getPeso(contexto, a) + config.getEpsilon();
            total += peso[a];
        }

    double r = std::uniform_real_distribution<double>(0.0, total)(generador);
    int elegido = 0;
    for (int a = 0; a < NUM_ATAQUES; a++) {
        if (!permitido[a]) continue;
        elegido = a;
        if (r < peso[a]) break;
        r -= peso[a];
    }
    ataque = elegido;

    objetivoX = percepcion->getXRick();
    int p = config.getPrediccion();
    if (p == 2 || (p == 1 && ataque == ATAQUE_RACIMOS)) {
        double t = ataque == ATAQUE_RACIMOS ? config.getTiempoVueloRacimo() : 0.5;
        objetivoX += percepcion->getVelX() * t;
        if (percepcion->getEvasionPreferida() == EVASION_PORTAL)
            objetivoX += percepcion->getVelX() < 0 ? -220 : 220;
    }
    if (objetivoX < 40) objetivoX = 40;
    if (objetivoX > 760) objetivoX = 760;
}

int ModuloRazonamiento::getAtaque() const { return ataque; }
double ModuloRazonamiento::getObjetivoX() const { return objetivoX; }
int ModuloRazonamiento::getContexto() const { return contexto; }
