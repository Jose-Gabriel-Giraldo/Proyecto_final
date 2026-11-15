#include "mainwindow.h"
#include "./ui_mainwindow.h"
#include "vistaplantacion.h"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    setWindowTitle("Rick y Morty: La anomalía de las bananeras");
    ui->menubar->hide();
    ui->statusbar->hide();

    // Por ahora la ventana muestra directamente el nivel 1.
    // La vista queda como hija de la ventana: Qt la libera al cerrar.
    VistaPlantacion* vista = new VistaPlantacion(this);
    setCentralWidget(vista);
    vista->setFocus();
    adjustSize();
}

MainWindow::~MainWindow()
{
    delete ui;
}
