#ifndef ENTIDAD_H
#define ENTIDAD_H

class ModeloFisico;

class Entidad {
public:
    Entidad(double x, double y, double ancho, double alto, ModeloFisico* fisica = nullptr);
    virtual ~Entidad();
    Entidad(const Entidad&) = delete;
    Entidad& operator=(const Entidad&) = delete;

    virtual void actualizar(double dt) = 0;
    bool colisionaCon(const Entidad& o) const;

    double getX() const;
    double getY() const;
    double getVx() const;
    double getVy() const;
    double getAncho() const;
    double getAlto() const;
    double getCentroX() const;
    double getCentroY() const;
    bool estaActiva() const;
    ModeloFisico* getFisica() const;

    void setPosicion(double nx, double ny);
    void setVelocidad(double nvx, double nvy);
    void setFisica(ModeloFisico* nueva);
    void desactivar();

protected:
    double x, y, vx, vy, ancho, alto;
    bool activa;
    ModeloFisico* fisica;
};

#endif
