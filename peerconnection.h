#ifndef PEERCONNECTION_H
#define PEERCONNECTION_H
#include "SharedQueue.h"
#include "bittorrentmessage.h"
#include "peerretriever.h"
#include "piecemanager.h"
#include <cstdint>
#include <initializer_list>
#include <ios>
#include <istream>
#include <list>
#include <map>
#include <memory>
#include <regex>
#include <sstream>
#include <string>

using byte = unsigned char;

class PeerConnection {
    private:
    int sock{};              // Сокет для соединения с пиром
    bool choked = true;      // Пир заблокирован передачей данных
    bool terminated = false; // Признак завершения соединения
    bool requestPending = false; // Флаг, указывающий, ожидается ли ответ на запрос к пиру
    const std::string clientId; // Идентификатор клиента
    const std::string infoHash; // Хэш информации
    SharedQueue<Peer *> *queue; // Очередь для обработки пиров
    Peer *peer;                 // Пир, с которым установлено соединение
    std::string peerBitField;   // Битовое поле пира
    std::string peerId;         // Идентификатор пира
    PieceManager *pieceManager; // Менеджер кусков файла

                                // Методы для управления соединением
    std::string createHandshakeMessage(); // Создание сообщения рукопожатия
    void performHandshake();              // Выполнение рукопожатия
    void receiveBitField();               // Получение битового поля от пира
    void sendInterested(); // Отправка сообщения о заинтересованности пиру
    void receiveUnchoke(); // Получение разблокировки от пира
    void requestPiece();   // Запрос куска файла у пира
    void closeSock();      // Закрытие сокета соединения
    bool establishNewConnection();                              // Установка нового соединения
    BitTorrentMessage receiveMessage(int bufferSize = 0) const; // Получение сообщения от пира

    public:
    const std::string &getPeerId() const;                // Получение идентификатора пира

    explicit PeerConnection(SharedQueue<Peer *> *queue,
                            std::string clientId,
                            std::string infoHash,
                            PieceManager *pieceManager); // Конструктор класса
    ~PeerConnection();                                   // Деструктор класса
    void start();                                        // Метод запуска соединения
    void stop();                                         // Метод завершения соединения
};

#endif                                                   // PEERCONNECTION_H
