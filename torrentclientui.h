#ifndef TORRENTCLIENTUI_H
#define TORRENTCLIENTUI_H

#include "torrentclient.h"
#include "torrentfile.h"
#include <QFileDialog>
#include <QLabel>
#include <QLineEdit>
#include <QPlainTextEdit>
#include <QPushButton>
#include <QVBoxLayout>
#include <QWidget>

class ConsoleRedirect : public QObject {
    Q_OBJECT
    public:
    explicit ConsoleRedirect(QObject *parent = nullptr) : QObject(parent)
    {
    }

    public slots:
    void write(const QString &text)
    {
        emit textWritten(text);
    }

    signals:
    void textWritten(const QString &text);
};

class TorrentClientUI : public QWidget {
    Q_OBJECT

    public:
    TorrentClientUI(QWidget *parent = nullptr); // Конструктор
    ~TorrentClientUI();

    private slots:
    void browseTorrentFile();                   // Слот для выбора .torrent файла
    void downloadTorrent(); // Слот для запуска загрузки торрент-файла
    void updateConsole(const QString &text); // Слот для обновления вывода в консоли
    void stopDownload();                     // Слот для остановки загрузки
    void chooseDownloadDirectory();

    private:
    TorrentClient client;
    QLineEdit *torrentFilePathLineEdit;      // Поле для отображения пути к .torrent файлу
    QLineEdit *downloadDirectoryLineEdit; // Поле для отображения пути к директории загрузки
    QLabel *torrentInfoLabel; // Добавлен для отображения информации о .torrent файле
    QPlainTextEdit *console;          // Консоль для отображения сообщений
    QPushButton *stopTorrentButton;   // Стоп загрузка

    ConsoleRedirect *consoleRedirect; // Объект перенаправления вывода
};

#endif                                // TORRENTCLIENTUI_H
