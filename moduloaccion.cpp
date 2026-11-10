#include "moduloaccion.h"
#include "bananamutante.h"
#include "configuraciondificultad.h"
#include "fisicaarrastre.h"
#include "fisicaparabolica.h"
#include "fisicapendular.h"
#include "moduloaprendizaje.h"
#include "modulorazonamiento.h"
#include "proyectil.h"
#include <cmath>

ModuloAccion::ModuloAccion(const ModuloRazonamiento& razonamiento, ModuloAprendizaje& aprendizaje, unsigned semilla)
    : razonamiento(&razonamiento), aprendizaje(&aprendizaje), siguienteId(0), generador(semilla) {}

int ModuloAccion::ejecutar(double xBoca, double yBoca, double ySuelo, int mutantesVivos,
                           const ConfiguracionDificultad& config, std::vector<Proyectil*>& proyectiles,
                           std::vector<BananaMutante*>& invocados) {
    int id = siguienteId++;
    int ataque = razonamiento->getAtaque();
    double objetivo = razonamiento->getObjetivoX();

    if (ataque == ATAQUE_BANANOS) {
        double v0 = 900, distancia = xBoca - objetivo;
        if (distancia < 100) distancia = 100;
        double c = v0 / distancia;
        if (c < 1.2) c = 1.2;
        if (c > 2.0) c = 2.0;
        for (double grados : {0.0, -12.0}) {
            double a = grados * PI / 180;
            proyectiles.push_back(new Proyectil(xBoca, ySuelo - 70, -v0 * std::cos(a), v0 * std::sin(a), 40, 20, 1,
                                                false, PROYECTIL_BANANO, new FisicaArrastre(c), id, 3.0));
        }
    } else if (ataque == ATAQUE_RACIMOS) {
        double T = config.getTiempoVueloRacimo(), g = 1400, vx, vy;
        FisicaParabolica::velocidadApuntado(objetivo - 25 - xBoca, ySuelo - 60 - yBoca, T, g, vx, vy);
        proyectiles.push_back(new Proyectil(xBoca, yBoca, vx, vy, 50, 60, 1, false, PROYECTIL_RACIMO,
                                            new FisicaParabolica(g), id, T + 1.0));
    } else if (ataque == ATAQUE_HOJAS) {
        int n = std::uniform_int_distribution<int>(3, 5)(generador);
        std::uniform_real_distribution<double> desvio(-320, 320), altura(0, 150), angulo(-0.6, 0.6);
        for (int i = 0; i < n; i++) {
            double xa = objetivo + desvio(generador);
            if (xa < 40) xa = 40;
            if (xa > 860) xa = 860;
            double ya = -100 - altura(generador);
            FisicaPendular* f = new FisicaPendular(xa, ya, 70, angulo(generador), 0.3, 1400, 240);
            proyectiles.push_back(new Proyectil(f->getPuntoX() - 13, f->getPuntoY() - 9, 0, 0, 26, 18, 1,
                                                false, PROYECTIL_HOJA, f, id, 8.0));
        }
    } else {
        int cantidad = 3 - mutantesVivos < 2 ? 3 - mutantesVivos : 2;
        for (int i = 0; i < cantidad; i++)
            invocados.push_back(new BananaMutante(xBoca - 80 - i * 80, ySuelo - 110, 40, 990, false, 110, 2000, 2));
    }

    aprendizaje->registrarPendiente(id, razonamiento->getContexto(), ataque);
    return id;
}
