#include "utils.h"

#include <bitset>
#include <cmath>
#include <iomanip>
#include <iostream>
#include <sstream>
#include <string>

// URL-кодирование строки
std::string urlEncode(const std::string &value)
{
    std::ostringstream escaped;
    escaped.fill('0');
    escaped << std::hex;

    for (char c : value)
    {
        if (isalnum(c) || c == '-' || c == '_' || c == '.' || c == '~')
        {
            escaped << c;
            continue;
        }

        escaped << std::uppercase;
        escaped << '%' << std::setw(2) << int((unsigned char)c);
        escaped << std::nouppercase;
    }

    return escaped.str();
}

// Декодирование строки из шестнадцатеричного формата
std::string hexDecode(const std::string &value)
{
    int hashLength = value.length();
    std::string decodedHexString;
    for (int i = 0; i < hashLength; i += 2)
    {
        std::string byte = value.substr(i, 2);
        char c = (char)(int)strtol(byte.c_str(), nullptr, 16);
        decodedHexString.push_back(c);
    }
    return decodedHexString;
}

// Кодирование строки в шестнадцатеричный формат
std::string hexEncode(const std::string &input)
{
    static const char hexDigits[] = "0123456789ABCDEF";

    std::string output;
    output.reserve(input.length() * 2);
    for (unsigned char c : input)
    {
        output.push_back('\\');
        output.push_back('x');
        output.push_back(hexDigits[c >> 4]);
        output.push_back(hexDigits[c & 15]);
    }
    return output;
}

// Проверка наличия куска в битовом поле
bool hasPiece(const std::string &bitField, int index)
{
    int byteIndex = floor(index / 8);
    int offset = index % 8;
    return (bitField[byteIndex] >> (7 - offset) & 1) != 0;
}

// Установка бита в битовом поле
void setPiece(std::string &bitField, int index)
{
    int byteIndex = floor(index / 8);
    int offset = index % 8;
    bitField[byteIndex] |= (1 << (7 - offset));
}

// Преобразование массива байтов в целое число
int bytesToInt(std::string bytes)
{
    std::string binStr;
    long byteCount = bytes.size();
    for (int i = 0; i < byteCount; i++)
    {
        binStr += std::bitset<8>(bytes[i]).to_string();
    }
    return stoi(binStr, 0, 2);
}

// Форматирование времени в формате HH:MM:SS
std::string formatTime(long seconds)
{
    if (seconds < 0)
    {
        return "inf";
    }
    std::string result;
    std::string h = std::to_string(seconds / 3600);
    std::string m = std::to_string((seconds % 3600) / 60);
    std::string s = std::to_string(seconds % 60);
    // Ведущие нули
    std::string hh = std::string(2 - h.length(), '0') + h;
    std::string mm = std::string(2 - m.length(), '0') + m;
    std::string ss = std::string(2 - s.length(), '0') + s;
    if (hh.compare("00") != 0)
    {
        result = hh + ':' + mm + ":" + ss;
    }
    else
    {
        result = mm + ":" + ss;
    }
    return result;
}
