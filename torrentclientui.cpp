#include <QApplication>
#include <QDebug> // Включаем это для qDebug
#include <QFileDialog>
#include <QIcon>
#include <QLabel>
#include <QLineEdit>
#include <QObject>
#include <QPlainTextEdit>
#include <QPushButton>
#include <QVBoxLayout>
#include <QWidget>

#include "torrentclient.h"
#include "torrentclientui.h"

TorrentClientUI::TorrentClientUI(QWidget *parent) : QWidget(parent)
{
    // Создаем экземпляр TorrentClient
    // Создаем поле для ввода пути к .torrent файлу
    torrentFilePathLineEdit = new QLineEdit(this);
    torrentFilePathLineEdit->setPlaceholderText("Путь к .torrent файлу");

    // Кнопка для выбора .torrent файла
    QPushButton *browseTorrentFileButton = new QPushButton(this);
    browseTorrentFileButton->setText("Выбрать .torrent файл");
    connect(browseTorrentFileButton, &QPushButton::clicked, this, &TorrentClientUI::browseTorrentFile);

    // Поле для ввода пути к директории загрузки
    downloadDirectoryLineEdit = new QLineEdit(this);
    downloadDirectoryLineEdit->setPlaceholderText("Директория загрузки");

    // Кнопка для выбора директории загрузки
    QPushButton *chooseDownloadDirButton = new QPushButton(this);
    chooseDownloadDirButton->setText("Выбрать папку");
    connect(chooseDownloadDirButton, &QPushButton::clicked, this, &TorrentClientUI::chooseDownloadDirectory);

    // Кнопка для начала загрузки торрент-файла
    QPushButton *downloadTorrentButton = new QPushButton(this);
    downloadTorrentButton->setText("Загрузить");
    connect(downloadTorrentButton, &QPushButton::clicked, this, &TorrentClientUI::downloadTorrent);

    // Кнопка для остановки загрузки торрент-файла
    stopTorrentButton = new QPushButton(this); // Инициализация кнопки остановки загрузки
    stopTorrentButton->setText("Стоп");
    stopTorrentButton->setEnabled(false);      // По умолчанию кнопка стоп неактивна
    connect(stopTorrentButton,
            &QPushButton::clicked,
            this,
            &TorrentClientUI::stopDownload); // Соединение сигнала остановки загрузки

    // Метка для отображения информации о выбранном .torrent файле
    torrentInfoLabel = new QLabel(this);

    // Консоль для вывода сообщений
    console = new QPlainTextEdit(this);
    console->setReadOnly(true);

    // Объект перенаправления
    consoleRedirect = new ConsoleRedirect(this);

    // Соединяем сигнал для перенаправления вывода в консоль
    connect(consoleRedirect, &ConsoleRedirect::textWritten, this, &TorrentClientUI::updateConsole);

    // Размещаем элементы управления на форме
    QVBoxLayout *layout = new QVBoxLayout;
    layout->addWidget(torrentFilePathLineEdit);
    layout->addWidget(browseTorrentFileButton);
    layout->addWidget(torrentInfoLabel);
    layout->addWidget(downloadDirectoryLineEdit);
    layout->addWidget(chooseDownloadDirButton); // Добавляем кнопку выбора папки загрузки в макет
    layout->addWidget(downloadTorrentButton); // Добавляем кнопку загрузки в макет
    layout->addWidget(stopTorrentButton); // Добавляем кнопку остановки загрузки в макет
    layout->addWidget(console);
    setLayout(layout);

                                          // Перенаправляем вывод в консоль
    qInstallMessageHandler([](QtMsgType type, const QMessageLogContext &context, const QString &msg) {
        QByteArray localMsg = msg.toLocal8Bit();
        switch (type)
        {
        case QtDebugMsg:
        case QtInfoMsg:
        case QtWarningMsg:
        case QtCriticalMsg:
        case QtFatalMsg:
            qDebug() << msg;
            break;
        }
        QCoreApplication::sendEvent(QCoreApplication::instance(),
                                    new QEvent(QEvent::UpdateRequest)); // Обновляем консоль
        emit static_cast<TorrentClientUI *>(QCoreApplication::instance()->parent())
            ->consoleRedirect->textWritten(
                QString::fromLocal8Bit(localMsg.constData())); // Передаем сигнал через родительский объект
    });
}

TorrentClientUI::~TorrentClientUI()
{
}

void TorrentClientUI::browseTorrentFile()
{
    // Диалоговое окно для выбора .torrent файла
    QString filePath =
        QFileDialog::getOpenFileName(this, "Выбрать .torrent файл", QDir::homePath(), "Торрент файлы (*.torrent)");
    if (!filePath.isEmpty())
    {
        torrentFilePathLineEdit->setText(filePath);

        // Отображаем информацию о выбранном .torrent файле
        try
        {
            TorrentFile torrent(filePath.toStdString());
            QString infoText = QString("Имя файла: %1\nРазмер файла: %2 байт\nТрекер: %3")
                                   .arg(torrent.getFileName().c_str())
                                   .arg(torrent.getFileSize())
                                   .arg(torrent.getAnnounce().c_str());
            torrentInfoLabel->setText(infoText);
        }
        catch (const std::exception &e)
        {
            qDebug() << "Ошибка: " << e.what();
            torrentInfoLabel->setText("Ошибка: Невозможно прочитать .torrent файл");
        }
    }
}

void TorrentClientUI::downloadTorrent()
{
    // Начинаем загрузку торрент-файла
    QString torrentFilePath = torrentFilePathLineEdit->text();
    QString downloadDirectory = downloadDirectoryLineEdit->text();
    if (!torrentFilePath.isEmpty() && !downloadDirectory.isEmpty())
    {
        if (!downloadDirectory.endsWith("/"))
        {
            downloadDirectory += "/";
        }

        client.downloadFile(torrentFilePath.toStdString(), downloadDirectory.toStdString());
        qDebug() << "Загрузка торрент файла из: " << torrentFilePath;
        qDebug() << "Загрузка в директорию: " << downloadDirectory;

        stopTorrentButton->setEnabled(true);
    }
    else
    {
        qDebug() << "Путь к торрент-файлу или директория загрузки пусты!";
    }
}

void TorrentClientUI::chooseDownloadDirectory()
{
    // Открыть диалоговое окно для выбора директории загрузки
    QString directory = QFileDialog::getExistingDirectory(this, "Выберите папку загрузки", QDir::homePath());

    // Если пользователь выбрал директорию, обновить поле ввода
    if (!directory.isEmpty())
    {
        downloadDirectoryLineEdit->setText(directory);
    }
}

void TorrentClientUI::stopDownload()
{
    // Останавливаем загрузку торрент-файла
    client.terminate();
    qDebug() << "Загрузка торрент-файла остановлена.";

    // Делаем кнопку стоп неактивной после остановки загрузки
    stopTorrentButton->setEnabled(false);
}

void TorrentClientUI::updateConsole(const QString &text)
{
    console->appendPlainText(text);
}
