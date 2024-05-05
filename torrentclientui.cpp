#include <QDebug>
#include <QDir>
#include <QFileDialog>
#include <QLabel>
#include <QLineEdit>
#include <QPushButton>
#include <QVBoxLayout>
#include <QWidget>

#include <QApplication>
#include <QIcon>

#include "torrentclient.h"
#include "torrentclientui.h"

TorrentClientUI::TorrentClientUI(QWidget *parent) : QWidget(parent)
{
    QVBoxLayout *layout = new QVBoxLayout(this);

    // Создание элементов интерфейса
    QLabel *titleLabel = new QLabel(
        "Добро пожаловать в Торрент-Клиент для Детей!\nЭтот простой и понятный опен-сурс торрент-клиент создан "
        "специально для детей,\nчтобы они могли безопасно и удобно загружать файлы из веб-паутины интернета.\nС "
        "его помощью вы сможете легко и быстро находить и загружать файлы, используя технологию торрентов.\nПросто "
        "выберите .torrent файл, который вы хотите загрузить, выберите каталог для загрузки файла,\nи нажмите "
        "кнопку <Загрузить торрент>. Этот клиент автоматически начнет загружать ваш файл."
        "\nКроме того, здесь специально добавлены две кнопки для вашего удобства!",
        this);
    layout->addWidget(titleLabel);

    QPushButton *browseButton = new QPushButton("Выбрать .torrent файл", this);
    layout->addWidget(browseButton);
    connect(browseButton, &QPushButton::clicked, this, &TorrentClientUI::browseTorrentFile);

    torrentFilePathLineEdit = new QLineEdit(this);
    layout->addWidget(torrentFilePathLineEdit);

    QPushButton *downloadButton = new QPushButton("Загрузить торрент", this);
    layout->addWidget(downloadButton);
    connect(downloadButton, &QPushButton::clicked, this, &TorrentClientUI::downloadTorrent);

    setLayout(layout);
}

void TorrentClientUI::browseTorrentFile()
{
    // Диалоговое окно выбора .torrent файла
    QString filePath =
        QFileDialog::getOpenFileName(this, "Выберите .torrent файл", QDir::homePath(), "Torrent Files (*.torrent)");
    if (!filePath.isEmpty())
    {
        torrentFilePathLineEdit->setText(filePath);
    }
}

void TorrentClientUI::downloadTorrent()
{
    // Запуск загрузки торрент-файла
    QString torrentFilePath = torrentFilePathLineEdit->text();
    QString downloadDirectory =
        QFileDialog::getExistingDirectory(this, "Выберите каталог для загрузки файла", QDir::homePath());

    if (!torrentFilePath.isEmpty() && !downloadDirectory.isEmpty())
    {
        if (!downloadDirectory.endsWith("/"))
        {
            downloadDirectory += "/";
        }

        TorrentClient client;
        client.downloadFile(torrentFilePath.toStdString(), downloadDirectory.toStdString());
        qDebug() << "Downloading torrent file from: " << torrentFilePath;
        qDebug() << "Downloading to directory: " << downloadDirectory;
    }
    else
    {
        qDebug() << "Torrent file path or download directory is empty!";
    }
}
