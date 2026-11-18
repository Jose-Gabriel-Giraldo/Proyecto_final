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

Proyectil* ModuloAccion::crearRacimo(double xBoca, double yBoca, double xObjetivo, double ySuelo, double T, int id) {
    double g = 1400, vx, vy;
    FisicaParabolica::velocidadApuntado(xObjetivo - xBoca, (ySuelo - 60) - (yBoca - 30), T, g, vx, vy);
    return new Proyectil(xBoca - 30, yBoca - 30, vx, vy, 60, 60, 1, false, PROYECTIL_RACIMO, new FisicaParabolica(g), id,
                         T + 1.0);
}

int ModuloAccion::ejecutar(double xBoca, double yBoca, double ySuelo, int mutantesVivos,
                           const ConfiguracionDificultad& config, std::vector<Proyectil*>& proyectiles,
                           std::vector<BananaMutante*>& invocados) {
    int id = siguienteId++;
    int ataque = razonamiento->getAtaque();
    double objetivo = razonamiento->getObjetivoX();

    if (ataque == ATAQUE_BANANOS) {
        int libre = std::uniform_int_distribution<int>(0, 2)(generador);
        for (int i = 0; i < 3; i++) {
            if (i == libre) continue;
            double dx = objetivo + (i - 1) * 200 - xBoca, dy = ySuelo - 60 - yBoca, d = std::hypot(dx, dy);
            double c = 0.8, v0 = d * c * 1.5;
            proyectiles.push_back(new Proyectil(xBoca - 18, yBoca - 18, v0 * dx / d, v0 * dy / d, 36, 36, 1, false,
                                                PROYECTIL_BANANO, new FisicaArrastre(c), id, 5.0));
        }
    } else if (ataque == ATAQUE_RACIMOS) {
        for (double desvio : {-130.0, 130.0})
            proyectiles.push_back(crearRacimo(xBoca, yBoca, objetivo + desvio, ySuelo, config.getTiempoVueloRacimo(), id));
    } else if (ataque == ATAQUE_HOJAS) {
        int n = std::uniform_int_distribution<int>(3, 5)(generador);
        std::uniform_real_distribution<double> desvio(-320, 320), altura(0, 150), angulo(-0.6, 0.6);
        for (int i = 0; i < n; i++) {
            double xa = objetivo + desvio(generador);
            if (xa < 40) xa = 40;
            if (xa > 1240) xa = 1240;
            double ya = -100 - altura(generador);
            FisicaPendular* f = new FisicaPendular(xa, ya, 70, angulo(generador), 0.3, 1400, 240);
            proyectiles.push_back(new Proyectil(f->getPuntoX() - 13, f->getPuntoY() - 9, 0, 0, 26, 18, 1,
                                                false, PROYECTIL_HOJA, f, id, 8.0));
        }
    } else {
        int cantidad = 3 - mutantesVivos < 2 ? 3 - mutantesVivos : 2;
        for (int i = 0; i < cantidad; i++) {
            double x = i % 2 == 0 ? 0 : 1220;
            invocados.push_back(new BananaMutante(x, ySuelo - 110, 0, 1220, false, 110, 2000, 2));
        }
    }

    aprendizaje->registrarPendiente(id, razonamiento->getContexto(), ataque);
    return id;
}
