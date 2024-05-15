#ifndef CONNECT_H
#define CONNECT_H
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

#include "log.h"

// Создание TCP-соединения с указанным IP-адресом и портом
int createConnection(const std::string &ip, int port);

// Отправка данных по указанному сокету
void sendData(int sock, const std::string &data);

// Получение данных из указанного сокета
std::string receiveData(int sock, uint32_t bufferSize = 0);

#endif // CONNECT_H
