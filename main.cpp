#include "torrentclientui.h"
#include <QApplication>
#include <QDebug>
#include <QFileInfo>
#include <QIcon>

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);

    QString iconPath = "/Users/ilushka/TORRENT/torrent-client/logo_rb.png";
    if (QFileInfo(iconPath).exists())
    {
        app.setWindowIcon(QIcon(iconPath));
    }
    else
    {
        qDebug() << "Иконка не найдена по пути:" << iconPath;
    }

    TorrentClientUI window;
    window.setWindowTitle("Торрент-клиент");
    window.resize(400, 200);
    window.show();

    return app.exec();
}
