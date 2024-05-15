#include "SharedQueue.h"
#include <cassert>
#include <chrono>
#include <cstring>
#include <iostream>
#include <netinet/in.h>
#include <stdexcept>
#include <unistd.h>
#include <utility>

#include "connect.h" // Включение заголовочного файла для соединения.
#include "peerconnection.h" // Включение заголовочного файла для соединения с пирами.
#include "utils.h"          // Включение загоеловочного файла утилит.

#define INFO_HASH_STARTING_POS 28 // Начальная позиция хэша информации в сообщении рукопожатия.
#define PEER_ID_STARTING_POS 48 // Начальная позиция идентификатора пира в сообщении рукопожатия.
#define HASH_LEN 20             // Длина хэша в байтах.
#define DUMMY_PEER_IP "0.0.0.0" // Фиктивный IP-адрес для проверки.

PeerConnection::PeerConnection(SharedQueue<Peer *> *queue,
                               std::string clientId,
                               std::string infoHash,
                               PieceManager *pieceManager)
    : queue(queue), clientId(std::move(clientId)), infoHash(std::move(infoHash)), pieceManager(pieceManager)
{
}

PeerConnection::~PeerConnection()
{
    closeSock();
    std::cout << "Поток загрузки завершен" << std::endl;
}

void PeerConnection::start()
{
    std::cout << "Запуск потока загрузки..." << std::endl;
    while (!(terminated || pieceManager->isComplete()))
    {
        peer = queue->pop_front();
        if (peer->ip == DUMMY_PEER_IP)
        {
            return;
        }

        try
        {
            if (establishNewConnection())
            {
                while (!pieceManager->isComplete())
                {
                    BitTorrentMessage message = receiveMessage();
                    if (message.getMessageId() > 10)
                    {
                        throw std::runtime_error("Получен недопустимый идентификатор сообщения от пира " + peerId);
                    }
                    switch (message.getMessageId())
                    {
                    case choke:
                        choked = true;
                        break;

                    case unchoke:
                        choked = false;
                        break;

                    case piece: {
                        requestPending = false;
                        std::string payload = message.getPayload();
                        int index = bytesToInt(payload.substr(0, 4));
                        int begin = bytesToInt(payload.substr(4, 4));
                        std::string blockData = payload.substr(8);
                        pieceManager->blockReceived(peerId, index, begin, blockData);
                        break;
                    }
                    case have: {
                        std::string payload = message.getPayload();
                        int pieceIndex = bytesToInt(payload);
                        pieceManager->updatePeer(peerId, pieceIndex);
                        break;
                    }

                    default:
                        break;
                    }
                    if (!choked)
                    {
                        if (!requestPending)
                        {
                            requestPiece();
                        }
                    }
                }
            }
        }
        catch (std::exception &e)
        {
            closeSock();
            std::cerr << "Произошла ошибка при загрузке от пира " << peerId << " [" << peer->ip << "]" << std::endl;
            std::cerr << e.what() << std::endl;
        }
    }
}

void PeerConnection::stop()
{
    terminated = true;
}

void PeerConnection::performHandshake()
{
    std::cout << "Подключение к пиру [" << peer->ip << "]..." << std::endl;
    try
    {
        sock = createConnection(peer->ip, peer->port);
    }
    catch (std::runtime_error &e)
    {
        throw std::runtime_error("Невозможно подключиться к пиру [" + peer->ip + "]");
    }
    std::cout << "Установлено TCP-соединение с пиром по сокету " << sock << ": УСПЕШНО" << std::endl;

    std::cout << "Отправка сообщения рукопожатия пиру [" << peer->ip << "]..." << std::endl;
    std::string handshakeMessage = createHandshakeMessage();
    sendData(sock, handshakeMessage);
    std::cout << "Отправлено сообщение рукопожатия: УСПЕШНО" << std::endl;

    std::cout << "Получение ответа на сообщение рукопожатия от пира [" << peer->ip << "]..." << std::endl;
    std::string reply = receiveData(sock, handshakeMessage.length());
    if (reply.empty())
    {
        throw std::runtime_error("Получение рукопожатия от пира: НЕ УДАЛОСЬ [Нет ответа от пира]");
    }
    peerId = reply.substr(PEER_ID_STARTING_POS, HASH_LEN);
    std::cout << "Получен ответ на сообщение рукопожатия от пира: УСПЕШНО" << std::endl;

    std::string receivedInfoHash = reply.substr(INFO_HASH_STARTING_POS, HASH_LEN);
    if ((receivedInfoHash == infoHash) != 0)
    {
        throw std::runtime_error("Выполнение рукопожатия с пиром " + peer->ip +
                                 ": НЕ УДАЛОСЬ [Получен несовпадающий хэш информации]");
    }
    std::cout << "Сравнение хэшей: УСПЕШНО" << std::endl;
}

void PeerConnection::receiveBitField()
{
    std::cout << "Получение сообщения BitField от пира [" << peer->ip << "]..." << std::endl;
    BitTorrentMessage message = receiveMessage();
    if (message.getMessageId() != bitField)
    {
        throw std::runtime_error("Получение BitField от пира: НЕ УДАЛОСЬ [Неверный идентификатор сообщения]");
    } // Возможна ошибка
    peerBitField = message.getPayload();

    pieceManager->addPeer(peerId, peerBitField);
    std::cout << "Получено сообщение BitField от пира: УСПЕШНО" << std::endl;
}

void PeerConnection::requestPiece()
{
    Block *block = pieceManager->nextRequest(peerId);

    if (!block)
    {
        return;
    }
    int payloadLength = 12;
    char temp[payloadLength];
    uint32_t index = htonl(block->piece);
    uint32_t offset = htonl(block->offset);
    uint32_t length = htonl(block->length);
    std::memcpy(temp, &index, sizeof(int));
    std::memcpy(temp + 4, &offset, sizeof(int));
    std::memcpy(temp + 8, &length, sizeof(int));
    std::string payload;
    for (int i = 0; i < payloadLength; i++)
    {
        payload += (char)temp[i];
    }

    std::stringstream info;
    info << "Отправка сообщения запроса пиру " << peer->ip << " ";
    info << "[Кусок: " << std::to_string(block->piece) << " ";
    info << "Смещение: " << std::to_string(block->offset) << " ";
    info << "Длина: " << std::to_string(block->length) << "]";
    std::cout << info.str() << std::endl;
    std::string requestMessage = BitTorrentMessage(request, payload).toString();
    sendData(sock, requestMessage);
    requestPending = true;
    std::cout << "Отправлено сообщение запроса: УСПЕШНО" << std::endl;
}

void PeerConnection::sendInterested()
{
    std::cout << "Отправка сообщения Interested пиру [" << peer->ip << "]..." << std::endl;
    std::string interestedMessage = BitTorrentMessage(interested).toString();
    sendData(sock, interestedMessage);
    std::cout << "Отправлено сообщение Interested: УСПЕШНО" << std::endl;
}

void PeerConnection::receiveUnchoke()
{
    std::cout << "Получение сообщения Unchoke от пира [" << peer->ip << "]..." << std::endl;
    BitTorrentMessage message = receiveMessage();
    if (message.getMessageId() != unchoke)
    {
        throw std::runtime_error("Получение сообщения Unchoke от пира: НЕ УДАЛОСЬ [Неверный идентификатор сообщения: " +
                                 std::to_string(message.getMessageId()) + "]");
    }
    choked = false;
    std::cout << "Получено сообщение Unchoke: УСПЕШНО" << std::endl;
}

bool PeerConnection::establishNewConnection()
{
    try
    {
        performHandshake();
        receiveBitField();
        sendInterested();
        return true;
    }
    catch (const std::runtime_error &e)
    {
        std::cerr << "Произошла ошибка при подключении к пиру [" << peer->ip << "]" << std::endl;
        std::cerr << e.what() << std::endl;
        return false;
    }
}

std::string PeerConnection::createHandshakeMessage()
{
    const std::string protocol = "BitTorrent protocol";
    std::stringstream buffer;
    buffer << (char)protocol.length();
    buffer << protocol;
    std::string reserved;
    for (int i = 0; i < 8; i++)
    {
        reserved.push_back('\0');
    }
    buffer << reserved;
    buffer << hexDecode(infoHash);
    buffer << clientId;
    assert(buffer.str().length() == protocol.length() + 49);
    return buffer.str();
}

BitTorrentMessage PeerConnection::receiveMessage(int bufferSize) const
{
    std::string reply = receiveData(sock, 0);
    if (reply.empty())
    {
        return BitTorrentMessage(keepAlive);
    }
    auto messageId = (uint8_t)reply[0];
    std::string payload = reply.substr(1);
    std::cout << "Получено сообщение с ID " << (int)messageId << " от пира [" << peer->ip << "]" << std::endl;
    return BitTorrentMessage(messageId, payload);
}

const std::string &PeerConnection::getPeerId() const
{
    return peerId;
}

void PeerConnection::closeSock()
{
    if (sock)
    {
        close(sock);
        sock = {};
        requestPending = false;
        if (!peerBitField.empty())
        {
            peerBitField.clear();
            pieceManager->removePeer(peerId);
        }
    }
}
