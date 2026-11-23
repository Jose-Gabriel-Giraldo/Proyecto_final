#ifndef CINEMATICA_H
#define CINEMATICA_H

#include <QPixmap>
#include <QString>
#include <QTimer>
#include <QWidget>
#include <utility>
#include <vector>

struct LineaDialogo {
    QString quien;
    QString texto;
};

class QPainter;

class Cinematica : public QWidget {
    Q_OBJECT
public:
    Cinematica(const QString& titulo, const QString& fondo, const std::vector<LineaDialogo>& lineas,
               const QString& objetivo = "", const std::vector<std::pair<QString, QString>>& controles = {},
               const QString& tituloPanel = "CONTROLES", QWidget* parent = nullptr);

    void iniciar();

    static std::vector<LineaDialogo> anomalia();
    static std::vector<LineaDialogo> palmeraMadre();
    static std::vector<LineaDialogo> epilogo();
    static std::vector<std::pair<QString, QString>> controlesMorty();
    static std::vector<std::pair<QString, QString>> controlesRick();

signals:
    void terminada();

protected:
    void paintEvent(QPaintEvent*) override;
    void keyPressEvent(QKeyEvent* e) override;

private:
    void avanzar();
    void dibujarControles(QPainter& p);

    QString titulo;
    QPixmap cielo, fondo, rick, morty;
    std::vector<LineaDialogo> lineas;
    QString objetivo, tituloPanel;
    std::vector<std::pair<QString, QString>> controles;
    int actual;
    bool mostrandoControles;
    double letras, tiempo;
    QTimer timer;
};

#endif
