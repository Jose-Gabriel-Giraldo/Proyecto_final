#include "palmerajefe.h"
#include "configuraciondificultad.h"
#include "moduloaccion.h"
#include "moduloaprendizaje.h"
#include "modulopercepcion.h"
#include "modulorazonamiento.h"

PalmeraJefe::PalmeraJefe(double x, double y, const Rick& rick, const ConfiguracionDificultad& config, unsigned semilla)
    : Personaje(x, y, 260, 440, 5000), fase(1), config(&config), aprendizaje(new ModuloAprendizaje()),
    percepcion(new ModuloPercepcion(rick)), razonamiento(new ModuloRazonamiento(*percepcion, *aprendizaje, semilla)),
    accion(new ModuloAccion(*razonamiento, *aprendizaje, semilla + 1)), pendiente(false), tiempoPendiente(0) {
    duracionInvulnerable = 0;
}

PalmeraJefe::~PalmeraJefe() {
    delete accion;
    delete razonamiento;
    delete percepcion;
    delete aprendizaje;
}

void PalmeraJefe::actualizar(double dt) {
    actualizarInvulnerabilidad(dt);
    percepcion->observarFrame(dt);
}

Proyectil* PalmeraJefe::disparar() { return nullptr; }

void PalmeraJefe::recibirDanio(int d) {
    Personaje::recibirDanio(d);
    if (fase == 1 && vida <= vidaMax / 2) fase = 2;
}

void PalmeraJefe::preparar(long long tiempoMs, bool contratiempo, int mutantesVivos) {
    percepcion->percibir();
    razonamiento->decidir(fase, contratiempo, mutantesVivos, *config);
    pendiente = true;
    tiempoPendiente = tiempoMs;
}

int PalmeraJefe::lanzar(int mutantesVivos, std::vector<Proyectil*>& proyectiles,
                        std::vector<BananaMutante*>& invocados) {
    if (!pendiente) return -1;
    pendiente = false;
    return accion->ejecutar(x + 30, y + 130, y + alto, mutantesVivos, *config, proyectiles, invocados);
}

void PalmeraJefe::notificarResultado(int idAtaque, bool impacto) {
    aprendizaje->resolver(idAtaque, impacto, config->getTasaAprendizaje());
}

int PalmeraJefe::getFase() const { return fase; }
bool PalmeraJefe::tieneAtaquePendiente() const { return pendiente; }
long long PalmeraJefe::getTiempoPendiente() const { return tiempoPendiente; }
int PalmeraJefe::getAtaquePendiente() const { return razonamiento->getAtaque(); }
const ModuloPercepcion& PalmeraJefe::getPercepcion() const { return *percepcion; }
const ModuloAprendizaje& PalmeraJefe::getAprendizaje() const { return *aprendizaje; }
ModuloAprendizaje& PalmeraJefe::getAprendizaje() { return *aprendizaje; }
