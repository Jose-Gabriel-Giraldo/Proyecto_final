#ifndef MORTY_H
#define MORTY_H

#include "personaje.h"
#include <random>

class FisicaPendular;

class Morty : public Personaje {
public:
    Morty(double x, double y);

    void actualizar(double dt) override;
    Proyectil* disparar() override;

    void mover(int dir);
    void setCorrer(bool c);
    void saltar();
    void agarrarLiana(FisicaPendular* l);
    void soltarLiana();
    void setLiberando(bool l);
    void registrarLiberado();
    void setEnemigosCerca(int n);

    double calcularDispersion() const;
    bool estaEnValentia() const;
    bool estaCansado() const;
    bool estaLiberando() const;
    double getValor() const;
    int getLiberados() const;
    FisicaPendular* getLiana() const;

private:
    void sumarValor(double p);

    double valor, tiempoValentia, tiempoCansancio, recargaDisparo;
    int enemigosCerca, liberados, dirMovimiento;
    bool corriendo, liberando;
    FisicaPendular* liana;
    std::mt19937 generador;
};

#endif
