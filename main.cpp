#include <QApplication>
#include <QIcon>

#include "torrentclientui.h"

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);
    QIcon icon(":/logo_rb.PNG");
    app.setWindowIcon(icon);
    QApplication::setWindowIcon(QIcon(":/logo_rb.ico"));

    TorrentClientUI window;
    window.setWindowTitle("Торрент-клиент");
    window.resize(400, 200);
    window.show();

    return app.exec();
}
