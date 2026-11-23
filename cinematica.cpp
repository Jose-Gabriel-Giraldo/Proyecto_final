#include "cinematica.h"
#include <QKeyEvent>
#include <QLinearGradient>
#include <QPainter>
#include <stdexcept>

const double LETRAS_POR_SEGUNDO = 45;

Cinematica::Cinematica(const QString& titulo, const QString& fondo, const std::vector<LineaDialogo>& lineas,
                       const QString& objetivo, const std::vector<std::pair<QString, QString>>& controles,
                       const QString& tituloPanel, QWidget* parent)
    : QWidget(parent), titulo(titulo), cielo(":/cielo.png"), fondo(fondo), rick(":/rick_idle.png"), morty(":/morty_idle.png"),
    lineas(lineas), objetivo(objetivo), tituloPanel(tituloPanel), controles(controles), actual(0), mostrandoControles(false), letras(0),
    tiempo(0) {
    if (this->fondo.isNull()) throw std::runtime_error("No se encontro la imagen " + fondo.toStdString());
    if (lineas.empty()) throw std::invalid_argument("Una cinematica necesita al menos una linea");
    setMinimumSize(640, 360);
    setFocusPolicy(Qt::StrongFocus);
    connect(&timer, &QTimer::timeout, this, [this]() {
        tiempo += 0.03;
        letras += LETRAS_POR_SEGUNDO * 0.03;
        update();
    });
}

void Cinematica::iniciar() {
    actual = 0;
    mostrandoControles = false;
    letras = 0;
    tiempo = 0;
    timer.start(30);
    setFocus();
}

void Cinematica::avanzar() {
    if (!mostrandoControles && !controles.empty() && actual + 1 >= static_cast<int>(lineas.size()) &&
        letras >= lineas[actual].texto.size()) {
        mostrandoControles = true;
        return;
    }
    if (mostrandoControles) {
        timer.stop();
        emit terminada();
        return;
    }
    if (letras < lineas[actual].texto.size()) {
        letras = lineas[actual].texto.size();
        return;
    }
    if (actual + 1 < static_cast<int>(lineas.size())) {
        actual++;
        letras = 0;
        return;
    }
    timer.stop();
    emit terminada();
}

void Cinematica::keyPressEvent(QKeyEvent* e) {
    if (e->isAutoRepeat()) return;
    if (e->key() == Qt::Key_Return || e->key() == Qt::Key_Enter || e->key() == Qt::Key_Space) avanzar();
    else if (e->key() == Qt::Key_Escape) {
        if (!controles.empty() && !mostrandoControles) {
            mostrandoControles = true;
            return;
        }
        timer.stop();
        emit terminada();
    } else QWidget::keyPressEvent(e);
}

void Cinematica::paintEvent(QPaintEvent*) {
    QPainter p(this);
    p.scale(width() / 1280.0, height() / 720.0);

    p.drawPixmap(QRectF(0, 0, 1280, 720), cielo, QRectF(cielo.rect()));
    double anchoFuente = fondo.height() * 16.0 / 9.0;
    if (anchoFuente > fondo.width()) anchoFuente = fondo.width();
    p.drawPixmap(QRectF(0, 0, 1280, 720), fondo, QRectF(0, 0, anchoFuente, fondo.height()));
    QLinearGradient sombra(0, 0, 0, 720);
    sombra.setColorAt(0, QColor(0, 0, 0, 120));
    sombra.setColorAt(1, QColor(0, 0, 0, 220));
    p.fillRect(QRectF(0, 0, 1280, 720), sombra);

    if (mostrandoControles) {
        dibujarControles(p);
        return;
    }

    const LineaDialogo& linea = lineas[actual];
    bool hablaRick = linea.quien == "RICK";
    double altoRick = 380, altoMorty = 330;
    double anchoRick = altoRick * rick.width() / rick.height(), anchoMorty = altoMorty * morty.width() / morty.height();
    p.setOpacity(hablaRick ? 0.45 : 1);
    p.drawPixmap(QRectF(170, 480 - altoMorty, anchoMorty, altoMorty), morty, QRectF(morty.rect()));
    p.setOpacity(hablaRick ? 1 : 0.45);
    p.save();
    p.translate(1110, 0);
    p.scale(-1, 1);
    p.drawPixmap(QRectF(0, 480 - altoRick, anchoRick, altoRick), rick, QRectF(rick.rect()));
    p.restore();
    p.setOpacity(1);

    p.setPen(QColor(255, 220, 120));
    p.setFont(QFont("Courier New", 22, QFont::Bold));
    p.drawText(QRectF(0, 24, 1280, 40), Qt::AlignCenter, titulo);

    QRectF caja(50, 490, 1180, 200);
    p.setPen(QPen(QColor(255, 255, 255, 180), 3));
    p.setBrush(QColor(10, 14, 20, 225));
    p.drawRoundedRect(caja, 14, 14);

    p.setPen(hablaRick ? QColor(110, 190, 255) : QColor(255, 210, 70));
    p.setFont(QFont("Courier New", 18, QFont::Bold));
    p.drawText(QRectF(80, 505, 400, 30), Qt::AlignLeft | Qt::AlignVCenter, linea.quien);

    int visibles = static_cast<int>(letras);
    if (visibles > linea.texto.size()) visibles = linea.texto.size();
    p.setPen(Qt::white);
    p.setFont(QFont("Courier New", 16));
    p.drawText(QRectF(80, 545, 1120, 130), Qt::AlignLeft | Qt::AlignTop | Qt::TextWordWrap, linea.texto.left(visibles));

    p.setFont(QFont("Courier New", 12));
    p.setPen(QColor(200, 200, 200));
    p.drawText(QRectF(80, 690, 600, 24), Qt::AlignLeft, "Esc: saltar");
    if (visibles >= linea.texto.size() && static_cast<int>(tiempo * 2) % 2 == 0)
        p.drawText(QRectF(600, 690, 600, 24), Qt::AlignRight, "Enter para continuar");
}

void Cinematica::dibujarControles(QPainter& p) {
    QRectF caja(240, 90, 800, 120 + 46 * controles.size());
    p.setPen(QPen(QColor(255, 220, 120), 3));
    p.setBrush(QColor(10, 14, 20, 235));
    p.drawRoundedRect(caja, 16, 16);

    p.setFont(QFont("Courier New", 26, QFont::Bold));
    p.drawText(QRectF(caja.x(), caja.y() + 18, caja.width(), 40), Qt::AlignCenter, tituloPanel);
    p.setPen(Qt::white);
    p.setFont(QFont("Courier New", 15));
    p.drawText(QRectF(caja.x() + 40, caja.y() + 62, caja.width() - 80, 40), Qt::AlignCenter | Qt::TextWordWrap,
               objetivo);

    for (int i = 0; i < static_cast<int>(controles.size()); i++) {
        double y = caja.y() + 115 + i * 46;
        p.setPen(Qt::NoPen);
        p.setBrush(QColor(255, 220, 120, 40));
        p.drawRoundedRect(QRectF(caja.x() + 60, y, 260, 36), 8, 8);
        p.setPen(QColor(255, 220, 120));
        p.setFont(QFont("Courier New", 16, QFont::Bold));
        p.drawText(QRectF(caja.x() + 60, y, 260, 36), Qt::AlignCenter, controles[i].first);
        p.setPen(Qt::white);
        p.setFont(QFont("Courier New", 16));
        p.drawText(QRectF(caja.x() + 350, y, 400, 36), Qt::AlignLeft | Qt::AlignVCenter, controles[i].second);
    }

    p.setFont(QFont("Courier New", 14, QFont::Bold));
    p.setPen(QColor(200, 200, 200));
    if (static_cast<int>(tiempo * 2) % 2 == 0)
        p.drawText(QRectF(0, caja.y() + caja.height() + 20, 1280, 30), Qt::AlignCenter, "Presiona Enter para continuar");
}

std::vector<std::pair<QString, QString>> Cinematica::controlesMorty() {
    return {{"A / D", "Moverse"},
            {"W / Espacio", "Saltar"},
            {"Shift", "Correr"},
            {"J / X", "Disparar"},
            {"E", "Agarrar o soltar liana"},
            {"Mantener E", "Abrir una jaula"},
            {"P", "Pausa"}};
}

std::vector<std::pair<QString, QString>> Cinematica::controlesRick() {
    return {{"A / D", "Moverse"},
            {"W / Espacio", "Saltar a las plataformas"},
            {"Mouse", "Apuntar"},
            {"Click izquierdo", "Disparar (mantener)"},
            {"K / Shift", "Portal (recarga 3 s)"},
            {"P", "Pausa"}};
}

std::vector<LineaDialogo> Cinematica::anomalia() {
    return {
            {"MORTY", "¿P-por qué estamos en 1928, Rick? ¿Y por qué todo huele a banano?"},
            {"RICK", "Porque hace tres viajes se me cayó un frasco de suero mutagénico en esta línea temporal. Un detalle "
                     "menor, Morty. *eructo* Error de redondeo cósmico."},
            {"MORTY", "¿Un detalle menor? ¡Hay bananas con dientes caminando por ahí!"},
            {"RICK", "Contexto, Morty. Estamos en Ciénaga, Magdalena. Los trabajadores de la compañía bananera están en "
                     "huelga: piden que les paguen en plata y no en vales, y condiciones dignas. La compañía presiona y el "
                     "gobierno manda al ejército."},
            {"RICK", "Y alguien en el campamento militar encontró mi suero. Lo regaron en la plantación pensando que les "
                     "daría soldados que no se cansan ni dudan."},
            {"MORTY", "Y en cambio…"},
            {"RICK", "En cambio las bananas mutaron. Soldados banana, bananas mutantes y una anomalía temporal que se está "
                     "comiendo la historia."},
            {"MORTY", "¿Y los trabajadores?"},
            {"RICK", "Los mutantes los están encerrando en jaulas por toda la plantación. Tú vas a liberarlos. Mínimo seis, "
                     "Morty. Yo te espero en la estación del tren con el portal calibrado."},
            {"MORTY", "¿Y-yo solo? ¿Por qué no vas tú?"},
            {"RICK", "Porque alguien tiene que calibrar el portal y alguien tiene que ser el héroe de bajo presupuesto. "
                     "Adivina cuál eres tú. ¡Muévete!"},
            };
}

std::vector<LineaDialogo> Cinematica::palmeraMadre() {
    return {
            {"MORTY", "¡Lo logré, Rick! Los trabajadores están a salvo… más o menos."},
            {"RICK", "Bien hecho, Morty. Pero la anomalía no se cerró. *eructo* Mira el detector."},
            {"MORTY", "¿Qué es ese ruido? Suena como… ¿música?"},
            {"RICK", "Es la frecuencia de la anomalía. El suero se concentró en la planta madre de toda la plantación. "
                     "Absorbió tanto que ahora es… eso."},
            {"MORTY", "¡Es una palmera gigante con cara de odio!"},
            {"RICK", "La Palmera Madre. Vibra al ritmo de la anomalía: cada pulso es un ataque. Y lo peor: aprende. Si "
                     "esquivas siempre igual, te va a leer como un libro."},
            {"MORTY", "¿Y cómo la vencemos?"},
            {"RICK", "Tú no. Yo. Tengo una pistola de rayos, un portal de bolsillo y un ego del tamaño de esta plantación. "
                     "Si la derribo antes de que termine la canción, la anomalía se cierra."},
            {"RICK", "Su tronco es inmune a mis rayos, pero los racimos que lanza están cargados de suero. Revienta uno "
                     "en el aire y le va a doler de verdad."},
            {"MORTY", "¿Y si no?"},
            {"RICK", "Entonces 1928 se queda con bananas asesinas para siempre y los libros de historia se vuelven todavía "
                     "más raros. Quédate atrás, Morty."},
            };
}

std::vector<LineaDialogo> Cinematica::epilogo() {
    return {
            {"MORTY", "¡Cayó! ¡La Palmera Madre cayó, Rick!"},
            {"RICK", "Te dije que el ego era un arma, Morty. *eructo* La anomalía se está cerrando."},
            {"MORTY", "¿Y los trabajadores? ¿La huelga?"},
            {"RICK", "Eso no lo arregla ningún portal. Lo que pasó aquí en diciembre de 1928 es historia real y hay que "
                     "recordarla. Nosotros solo sacamos las bananas asesinas de la ecuación."},
            {"MORTY", "Entonces… ¿nos vamos?"},
            {"RICK", "Al portal, Morty. Y no toques nada que huela a banano."},
            };
}
