#ifndef TIPOS_H
#define TIPOS_H

const double PI = 3.14159265358979323846;

const int PROYECTIL_DISPARO_MORTY = 0;
const int PROYECTIL_BALA_SOLDADO = 1;
const int PROYECTIL_RAYO_RICK = 2;
const int PROYECTIL_BANANO = 3;
const int PROYECTIL_RACIMO = 4;
const int PROYECTIL_HOJA = 5;

const int NIVEL_EN_CURSO = 0;
const int NIVEL_GANADO = 1;
const int NIVEL_PERDIDO = 2;

const int ACCION_IZQUIERDA = 0;
const int ACCION_DERECHA = 1;
const int ACCION_SALTAR = 2;
const int ACCION_CORRER = 3;
const int ACCION_DISPARAR = 4;
const int ACCION_INTERACTUAR = 5;
const int ACCION_PORTAL = 6;

const int NUM_ATAQUES = 4;
const int ATAQUE_BANANOS = 0;
const int ATAQUE_RACIMOS = 1;
const int ATAQUE_HOJAS = 2;
const int ATAQUE_MUTANTES = 3;

const int NUM_EVASIONES = 4;
const int EVASION_NINGUNA = 0;
const int EVASION_SALTO = 1;
const int EVASION_PORTAL = 2;
const int EVASION_LATERAL = 3;

const int EVENTO_DISPARO = 0;
const int EVENTO_DANIO_JUGADOR = 1;
const int EVENTO_ENEMIGO_DERROTADO = 2;
const int EVENTO_TRABAJADOR_LIBERADO = 3;
const int EVENTO_ATAQUE_JEFE = 4;
const int EVENTO_CAMBIO_FASE = 5;

#endif
