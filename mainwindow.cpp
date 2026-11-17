#include "mainwindow.h"
#include "cinematica.h"
#include "vistajefe.h"
#include "vistaplantacion.h"
#include <QLabel>
#include <QMessageBox>
#include <QPushButton>
#include <QVBoxLayout>

MainWindow::MainWindow(QWidget* parent)
    : QMainWindow(parent), pantallas(new QStackedWidget(this)), menu(nullptr), actual(nullptr),
    config(ConfiguracionDificultad::normal()), puntajeNivel1(0), puntajeNivel2(0) {
    setWindowTitle("Rick & Morty: La Anomalía de las Bananeras");
    resize(1280, 720);
    setCentralWidget(pantallas);
    crearMenu();
}

void MainWindow::crearMenu() {
    menu = new QWidget;
    menu->setObjectName("menu");
    menu->setStyleSheet(
        "#menu { border-image: url(:/nivel2.png); }"
        "QLabel { color: #ffdc78; font: bold 20px 'Courier New'; }"
        "QPushButton { font: bold 20px 'Courier New'; color: white; background: rgba(10, 14, 20, 210);"
        " border: 2px solid #ffdc78; border-radius: 10px; padding: 12px; min-width: 300px; }"
        "QPushButton:hover { background: rgba(60, 130, 60, 230); }");

    QVBoxLayout* capa = new QVBoxLayout(menu);
    capa->addStretch();
    QLabel* titulo = new QLabel("Rick & Morty\nLa Anomalía de las Bananeras");
    titulo->setAlignment(Qt::AlignCenter);
    titulo->setStyleSheet("font: bold 46px 'Courier New';");
    capa->addWidget(titulo);
    capa->addSpacing(20);
    QLabel* elegir = new QLabel("Elige la dificultad");
    elegir->setAlignment(Qt::AlignCenter);
    capa->addWidget(elegir);

    for (const ConfiguracionDificultad& d :
         {ConfiguracionDificultad::facil(), ConfiguracionDificultad::normal(), ConfiguracionDificultad::dificil()}) {
        QPushButton* boton = new QPushButton(QString::fromStdString(d.getNombre()));
        capa->addWidget(boton, 0, Qt::AlignHCenter);
        connect(boton, &QPushButton::clicked, this, [this, d]() { empezar(d); });
    }
    QPushButton* salir = new QPushButton("Salir");
    capa->addWidget(salir, 0, Qt::AlignHCenter);
    connect(salir, &QPushButton::clicked, this, &QWidget::close);

    capa->addStretch();
    QLabel* controles = new QLabel("Morty: A/D moverse · W saltar · Shift correr · J disparar · E liana/jaula\n"
                                   "Rick: A/D moverse · W saltar · J disparar · K portal          P: pausa");
    controles->setAlignment(Qt::AlignCenter);
    controles->setStyleSheet("color: white; font: 14px 'Courier New';");
    capa->addWidget(controles);
    pantallas->addWidget(menu);
}

void MainWindow::mostrar(QWidget* pantalla) {
    pantallas->addWidget(pantalla);
    pantallas->setCurrentWidget(pantalla);
    if (actual) {
        pantallas->removeWidget(actual);
        actual->deleteLater();
    }
    actual = pantalla;
    pantalla->setFocus();
}

void MainWindow::volverAlMenu() {
    pantallas->setCurrentWidget(menu);
    if (actual) {
        pantallas->removeWidget(actual);
        actual->deleteLater();
        actual = nullptr;
    }
}

void MainWindow::error(const std::exception& e) {
    QMessageBox::critical(this, "Error", e.what());
    volverAlMenu();
}

void MainWindow::empezar(const ConfiguracionDificultad& dificultad) {
    config = dificultad;
    puntajeNivel1 = puntajeNivel2 = 0;
    cinematica1();
}

void MainWindow::cinematica1() {
    try {
        Cinematica* c = new Cinematica("La anomalía", ":/nivel1.png", Cinematica::anomalia(),
                                       "Libera al menos 6 trabajadores de sus jaulas y llega a la estación del tren.",
                                       Cinematica::controlesMorty());
        connect(c, &Cinematica::terminada, this, &MainWindow::nivel1);
        mostrar(c);
        c->iniciar();
    } catch (const std::exception& e) {
        error(e);
    }
}

void MainWindow::nivel1() {
    try {
        VistaPlantacion* v = new VistaPlantacion;
        connect(v, &VistaPlantacion::nivelTerminado, this, [this](bool, int puntaje) {
            puntajeNivel1 = puntaje;
            cinematica2();
        });
        connect(v, &VistaPlantacion::salirAlMenu, this, &MainWindow::volverAlMenu);
        mostrar(v);
        v->iniciar();
    } catch (const std::exception& e) {
        error(e);
    }
}

void MainWindow::cinematica2() {
    try {
        Cinematica* c = new Cinematica("La Palmera Madre", ":/nivel2.png", Cinematica::palmeraMadre(),
                                       "Derriba a la Palmera Madre antes de que termine la canción. Cuando la música "
                                       "está en calma es invulnerable.",
                                       Cinematica::controlesRick());
        connect(c, &Cinematica::terminada, this, &MainWindow::nivel2);
        mostrar(c);
        c->iniciar();
    } catch (const std::exception& e) {
        error(e);
    }
}

void MainWindow::nivel2() {
    try {
        VistaJefe* v = new VistaJefe(config);
        connect(v, &VistaJefe::nivelTerminado, this, [this](bool, int puntaje) {
            puntajeNivel2 = puntaje;
            epilogo();
        });
        connect(v, &VistaJefe::salirAlMenu, this, &MainWindow::volverAlMenu);
        mostrar(v);
        v->iniciar();
    } catch (const std::exception& e) {
        error(e);
    }
}

void MainWindow::epilogo() {
    try {
        std::vector<std::pair<QString, QString>> resumen = {
                                                            {"Nivel 1", QString::number(puntajeNivel1)},
                                                            {"Nivel 2", QString::number(puntajeNivel2)},
                                                            {"Total", QString::number(puntajeNivel1 + puntajeNivel2)},
                                                            {"Dificultad", QString::fromStdString(config.getNombre())}};
        Cinematica* c = new Cinematica("Epílogo", ":/nivel2.png", Cinematica::epilogo(),
                                       "Rick y Morty cerraron la anomalía de las bananeras.", resumen, "¡VICTORIA!");
        connect(c, &Cinematica::terminada, this, &MainWindow::volverAlMenu);
        mostrar(c);
        c->iniciar();
    } catch (const std::exception& e) {
        error(e);
    }
}
