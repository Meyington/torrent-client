#ifndef TORRENTCLIENTUI_H
#define TORRENTCLIENTUI_H

#include <QLineEdit>
#include <QWidget>

class TorrentClientUI : public QWidget {
    Q_OBJECT

    public:
    TorrentClientUI(QWidget *parent = nullptr); // Конструктор

    private slots:
    void browseTorrentFile();                   // Слот для выбора .torrent файла
    void downloadTorrent();             // Слот для запуска загрузки торрент-файла

    private:
    QLineEdit *torrentFilePathLineEdit; // Поле для отображения пути к .torrent файлу
};

#endif                                  // TORRENTCLIENTUI_H
