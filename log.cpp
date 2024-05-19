#include "log.h"
/*
#include <fstream>
#include <iostream>
#include <sstream>

#ifdef _WIN32
#include <direct.h>   // для Windows
#define MKDIR(directory) _mkdir(directory)
#else
#include <sys/stat.h> // для macOS и Linux
#define MKDIR(directory) mkdir(directory, 0777)
#endif

void Logger::createLogDirectory()
{
    // Создание директории для логов
    MKDIR(logDirectory.c_str());
}

void Logger::logMessage(const std::string &message)
{
    // Получение текущего времени
    std::string currentTime = getCurrentTime();

    // Формирование имени файла для логов
    std::ostringstream filenameStream;
    filenameStream << logDirectory << "/torrent_client_" << currentTime << ".txt";
    std::string logFilename = filenameStream.str();

    // Открытие файла для логов
    std::ofstream logfile(logFilename, std::ios::app);

    // Запись сообщения в файл
    if (logfile.is_open())
    {
        logfile << message << std::endl;
        logfile.close();
    }
    else
    {
        std::cerr << "Unable to open log file!" << std::endl;
    }
}

std::string Logger::getCurrentTime()
{
    // Получение текущего времени
    std::time_t now = std::time(nullptr);
    std::tm *localTime = std::localtime(&now);

    // Форматирование времени
    char timeString[100];
    std::strftime(timeString, sizeof(timeString), "%H:%M:%S_%d:%m:%Y", localTime);

    return std::string(timeString);
}
*/
