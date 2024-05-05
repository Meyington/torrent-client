#ifndef UTILS_H
#define UTILS_H

#include <string>

std::string urlEncode(const std::string &value); // URL-кодирование строки
std::string hexDecode(const std::string &value); // Декодирование строки из шестнадцатеричного формата
std::string hexEncode(const std::string &input); // Кодирование строки в шестнадцатеричный формат
bool hasPiece(const std::string &bitField, int index); // Проверка наличия куска в битовом поле
void setPiece(std::string &bitField, int index); // Установка бита в битовом поле
int bytesToInt(std::string bytes); // Преобразование массива байтов в целое число
std::string formatTime(long seconds); // Форматирование времени в формате HH:MM:SS

#endif                                // UTILS_H
