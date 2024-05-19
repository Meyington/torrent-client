#include "connect.h"
#include "utils.h"
#include <arpa/inet.h>
#include <chrono>
#include <cstring>
#include <fcntl.h>
#include <limits>
#include <netinet/in.h>
#include <stdexcept>
#include <sys/poll.h>
#include <sys/socket.h>
#include <unistd.h>

#define CONNECT_TIMEOUT 3
#define READ_TIMEOUT 3000 // 3 секунды

// Установка сокета в блокирующий или неблокирующий режим
bool setSocketBlocking(int sock, bool blocking)
{
    if (sock < 0)
    {
        return false;
    }
    int flags = fcntl(sock, F_GETFL, 0);
    if (flags == -1)
    {
        return false;
    }
    flags = blocking ? (flags & ~O_NONBLOCK) : (flags | O_NONBLOCK);
    return (fcntl(sock, F_SETFL, flags) == 0);
}

int createConnection(const std::string &ip, const int port)
{
    int sock = 0;
    struct sockaddr_in address;
    if ((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0)
    {
        throw std::runtime_error("Ошибка создания сокета: " + std::to_string(sock));
    }
    address.sin_family = AF_INET;
    address.sin_port = htons(port);

    char *tempIp = new char[ip.length() + 1];
    strcpy(tempIp, ip.c_str());

    // Преобразование IP-адреса из строки в структуру in_addr
    if (inet_pton(AF_INET, tempIp, &address.sin_addr) <= 0)
    {
        throw std::runtime_error("Некорректный IP-адрес: " + ip);
    }
    // Установка сокета в неблокирующий режим
    if (!setSocketBlocking(sock, false))
    {
        throw std::runtime_error("Произошла ошибка при установке сокета " + std::to_string(sock) + " в режим NONBLOCK");
    }
    connect(sock, (struct sockaddr *)&address, sizeof(address));

    fd_set fdset;
    struct timeval tv;
    FD_ZERO(&fdset);
    FD_SET(sock, &fdset);
    tv.tv_sec = CONNECT_TIMEOUT;
    tv.tv_usec = 0;

    if (select(sock + 1, NULL, &fdset, NULL, &tv) == 1)
    {
        int so_error;
        socklen_t len = sizeof so_error;

        getsockopt(sock, SOL_SOCKET, SO_ERROR, &so_error, &len);

        if (so_error == 0)
        {
            // Установка сокета в блокирующий режим
            if (!setSocketBlocking(sock, true))
            {
                throw std::runtime_error("Произошла ошибка при установке сокета " + std::to_string(sock) +
                                         "в режим BLOCK");
            }
            return sock;
        }
    }
    close(sock);
    throw std::runtime_error("Подключение к " + ip + ": НЕУДАЧА [Таймаут подключения]");
}

// Отправка данных по указанному сокету
void sendData(const int sock, const std::string &data)
{
    int n = data.length();
    char buffer[n];
    for (int i = 0; i < n; i++)
    {
        buffer[i] = data[i];
    }
    int res = send(sock, buffer, n, 0);
    if (res < 0)
    {
        throw std::runtime_error("Не удалось записать данные в сокет " + std::to_string(sock));
    }
}

// Получение данных из указанного сокета
std::string receiveData(const int sock, uint32_t bufferSize)
{
    std::string reply;

    // Если размер буфера не указан, считываем первые 4 байта сообщения
    // и интерпретируем их как целое число, указывающее общую длину сообщения.
    if (!bufferSize)
    {
        struct pollfd fd;
        int ret;
        fd.fd = sock;
        fd.events = POLLIN;
        ret = poll(&fd, 1, READ_TIMEOUT);

        long bytesRead;
        const int lengthIndicatorSize = 4;
        char buffer[lengthIndicatorSize];
        switch (ret)
        {
        case -1:
            throw std::runtime_error("Чтение из сокета " + std::to_string(sock) + " не удалось");
        case 0:
            throw std::runtime_error("Таймаут чтения из сокета " + std::to_string(sock));
        default:
            bytesRead = recv(sock, buffer, sizeof(buffer), 0);
        }
        if (bytesRead != lengthIndicatorSize)
        {
            return reply;
        }

        std::string messageLengthStr;
        for (char i : buffer)
        {
            messageLengthStr += i;
        }
        uint32_t messageLength = bytesToInt(messageLengthStr);
        bufferSize = messageLength;
    }

    // Если размер буфера больше максимального значения uint16_t, при инициализации буфера
    // произойдет сбой сегментации
    if (bufferSize > std::numeric_limits<uint16_t>::max())
    {
        throw std::runtime_error("Получены поврежденные данные [Размер буфера превышает 2 ^ 16 - 1]");
    }
    char buffer[bufferSize];
    long bytesRead = 0;
    long bytesToRead = bufferSize;
    auto startTime = std::chrono::steady_clock::now();
    do
    {
        auto diff = std::chrono::steady_clock::now() - startTime;
        if (std::chrono::duration<double, std::milli>(diff).count() > READ_TIMEOUT)
        {
            throw std::runtime_error("Таймаут чтения из сокета " + std::to_string(sock));
        }
        bytesRead = recv(sock, buffer, bufferSize, 0);

        if (bytesRead <= 0)
        {
            throw std::runtime_error("Не удалось получить данные из сокета " + std::to_string(sock));
        }
        bytesToRead -= bytesRead;
        for (int i = 0; i < bytesRead; i++)
        {
            reply.push_back(buffer[i]);
        }
    } while (bytesToRead > 0);

    return reply;
}
