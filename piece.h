#ifndef PIECE_H
#define PIECE_H

#include <memory>
#include <string>
#include <vector>

enum BlockStatus
{
    Missing = 0,                 // Фрагмент отсутствует в настоящий момент
    Pending,                     // Фрагмент ожидает загрузки
    Retrieved                    // Фрагмент загружен
};

struct Block
{
    int piece;                   // Индекс фрагмента, к которому принадлежит блок
    int offset;                  // Смещение блока внутри фрагмента
    int length;                  // Размер блока
    BlockStatus status;          // Состояние блока
    std::string data;            // Данные блока
};

class Piece {
    private:
    const std::string hashValue; // Хэш-значение для проверки целостности фрагмента

    public:
    const int index;             // Индекс фрагмента
    std::vector<Block *> blocks; // Список блоков, составляющих фрагмент

                                 // Конструктор класса Piece
    Piece(int index, std::vector<Block *> blocks, std::string hashValue);

    // Деструктор класса Piece
    ~Piece();

    // Сбрасывает состояние всех блоков фрагмента на Missing
    void reset();

    // Возвращает следующий блок для загрузки (состояние блока меняется на Pending)
    Block *nextRequest();

    // Устанавливает состояние блока в Retrieved и сохраняет полученные данные
    void blockReceived(int offset, std::string data);

    // Проверяет, загружены ли все блоки фрагмента
    bool isComplete();

    // Проверяет соответствие хэш-значения данных фрагмента ожидаемому значению
    bool isHashMatching();

    // Получает данные всех блоков фрагмента и объединяет их в одну строку
    std::string getData();
};

using PiecePtr = std::shared_ptr<Piece>;

#endif // PIECE_H
